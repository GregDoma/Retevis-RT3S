/*
 * Fork: field operations data in the SPI flash (see functions/fieldData.h).
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#include <string.h>
#include "functions/fieldData.h"
#include "functions/aprsText.h"

#if defined(HAS_FIELD_TOOLS)

#include "hardware/SPI_Flash.h"

#define SECTOR_SIZE      4096U
#define PAGE_SIZE        256U
#define PACE_ADDRESS     FIELD_DATA_FLASH_ADDRESS
#define PACE_MAGIC       0x45434150UL // "PACE"
#define COVERAGE_MAGIC   0x31564F43UL // "COV1": the coverage sectors were erased by this firmware
#define COVERAGE_ADDRESS (FIELD_DATA_FLASH_ADDRESS + SECTOR_SIZE)
#define REPLIES_ADDRESS  (FIELD_DATA_FLASH_ADDRESS + FIELD_APRS_REPLIES_OFFSET) // sector 0, after the PACE store
#define WAYPOINTS_ADDRESS (FIELD_DATA_FLASH_ADDRESS + FIELD_WAYPOINTS_OFFSET)  // sector 0, after the replies
#define SKEDS_ADDRESS    (FIELD_DATA_FLASH_ADDRESS + FIELD_SKEDS_OFFSET)       // sector 0, after the waypoints
#define WAYPOINT_MAGIC   0x31505757UL // "WWP1"
#define SKED_MAGIC       0x31444B53UL // "SKD1"

typedef struct
{
	uint32_t   magic;
	uint32_t   coverageMagic; // the area may hold old GPS log data until it is erased once
	paceSlot_t slots[PACE_SLOTS];
} paceStore_t;

static uint32_t coverageCount = 0xFFFFFFFFU; // unknown until the first access

__attribute__((section(".ccmram"))) uint32_t fieldToolsScratch[FIELD_SCRATCH_SIZE / sizeof(uint32_t)];

// --- PACE --------------------------------------------------------------------------------
static void paceRead(paceStore_t *store)
{
	SPI_Flash_read(PACE_ADDRESS, (uint8_t *)store, sizeof(paceStore_t));
	if (store->magic != PACE_MAGIC)
	{
		store->magic = PACE_MAGIC;
		store->coverageMagic = 0U;
		for (int i = 0; i < PACE_SLOTS; i++)
		{
			store->slots[i].zoneNumber = -1;
			store->slots[i].channelNumber = 0;
		}
	}
}

bool fieldPaceGet(int slot, paceSlot_t *data)
{
	paceStore_t store;

	if ((slot < 0) || (slot >= PACE_SLOTS))
	{
		return false;
	}

	paceRead(&store);
	*data = store.slots[slot];
	return ((data->zoneNumber >= 0) && (data->channelNumber > 0) && (data->channelNumber <= 1024));
}

_Static_assert(sizeof(paceStore_t) <= FIELD_APRS_REPLIES_OFFSET, "PACE store overlaps the APRS replies");
_Static_assert((FIELD_APRS_REPLIES_OFFSET + (APRS_REPLY_SLOTS * sizeof(aprsReplyRecord_t))) <= FIELD_WAYPOINTS_OFFSET, "APRS replies overlap the waypoints");
_Static_assert(sizeof(fieldWaypoint_t) == 24U, "waypoint record size (tools/opengd77_coverage.py)");
_Static_assert((FIELD_WAYPOINTS_OFFSET + (FIELD_WAYPOINT_SLOTS * sizeof(fieldWaypoint_t))) <= FIELD_SKEDS_OFFSET, "waypoints overlap the skeds");
_Static_assert(sizeof(fieldSked_t) == 16U, "sked record size");
_Static_assert((FIELD_SKEDS_OFFSET + (FIELD_SKED_SLOTS * sizeof(fieldSked_t))) <= SECTOR_SIZE, "skeds do not fit in sector 0");

// Sector 0 is shared (PACE store + APRS replies): read-modify-write of the whole
// sector through the SPI flash sector buffer, so the other records are kept.
static void storeWrite(const paceStore_t *store)
{
	SPI_Flash_write(PACE_ADDRESS, (uint8_t *)store, sizeof(paceStore_t));
}

void fieldPaceSet(int slot, const paceSlot_t *data)
{
	paceStore_t storeBuf;
	paceStore_t *store = &storeBuf;

	if ((slot < 0) || (slot >= PACE_SLOTS))
	{
		return;
	}

	paceRead(store);
	if (data != NULL)
	{
		store->slots[slot] = *data;
	}
	else
	{
		store->slots[slot].zoneNumber = -1;
		store->slots[slot].channelNumber = 0;
	}

	storeWrite(store);
}

// --- APRS canned replies ------------------------------------------------------------------
bool fieldAprsReplyGet(int slot, char *text)
{
	aprsReplyRecord_t rec;

	if ((slot < 0) || (slot >= APRS_REPLY_SLOTS))
	{
		text[0] = 0;
		return false;
	}

	SPI_Flash_read((REPLIES_ADDRESS + (slot * sizeof(aprsReplyRecord_t))), (uint8_t *)&rec, sizeof(rec));
	return aprsReplyRecordDecode(slot, &rec, text);
}

void fieldAprsReplySet(int slot, const char *text)
{
	aprsReplyRecord_t rec;

	if ((slot < 0) || (slot >= APRS_REPLY_SLOTS))
	{
		return;
	}

	// No-op saves do not rewrite sector 0 (it also holds the PACE store)
	char current[APRS_TEXT_MSG_MAX + 1];
	bool edited = fieldAprsReplyGet(slot, current);

	if ((text == NULL) || (text[0] == 0))
	{
		if (edited == false)
		{
			return; // already the default
		}
	}
	else if (strncmp(text, current, APRS_TEXT_MSG_MAX) == 0)
	{
		return; // unchanged (or the same as the default)
	}

	aprsReplyRecordEncode(text, &rec);
	SPI_Flash_write((REPLIES_ADDRESS + (slot * sizeof(aprsReplyRecord_t))), (uint8_t *)&rec, sizeof(rec));
}

// --- saved waypoints ----------------------------------------------------------------------
bool fieldWaypointGet(int slot, fieldWaypoint_t *wp)
{
	if ((slot < 0) || (slot >= FIELD_WAYPOINT_SLOTS))
	{
		return false;
	}

	SPI_Flash_read((WAYPOINTS_ADDRESS + (slot * sizeof(fieldWaypoint_t))), (uint8_t *)wp, sizeof(fieldWaypoint_t));
	if ((wp->magic != WAYPOINT_MAGIC) || (wp->lat > 90000000) || (wp->lat < -90000000) || (wp->lon > 180000000) || (wp->lon < -180000000))
	{
		return false;
	}

	for (int i = 0; i < FIELD_WAYPOINT_NAME_LEN; i++)
	{
		char c = wp->name[i];

		if ((c != 0) && ((c < ' ') || (c > '~')))
		{
			return false;
		}
	}
	return true;
}

void fieldWaypointSet(int slot, const fieldWaypoint_t *wp)
{
	fieldWaypoint_t rec;

	if ((slot < 0) || (slot >= FIELD_WAYPOINT_SLOTS))
	{
		return;
	}

	if (wp != NULL)
	{
		rec = *wp;
		rec.magic = WAYPOINT_MAGIC;
	}
	else
	{
		memset(&rec, 0xFF, sizeof(rec));
	}
	// Sector 0 read-modify-write (SPI_Flash_write): the PACE store, replies and skeds are kept
	SPI_Flash_write((WAYPOINTS_ADDRESS + (slot * sizeof(fieldWaypoint_t))), (uint8_t *)&rec, sizeof(rec));
}

int fieldWaypointAdd(double lat, double lon, const char *name, uint8_t flags)
{
	fieldWaypoint_t wp;

	for (int i = 0; i < FIELD_WAYPOINT_SLOTS; i++)
	{
		if (fieldWaypointGet(i, &wp) == false)
		{
			memset(&wp, 0, sizeof(wp));
			wp.lat = (int32_t)((lat * 1e6) + ((lat < 0.0) ? -0.5 : 0.5));
			wp.lon = (int32_t)((lon * 1e6) + ((lon < 0.0) ? -0.5 : 0.5));
			wp.flags = flags;
			if ((name != NULL) && (name[0] != 0))
			{
				strncpy(wp.name, name, FIELD_WAYPOINT_NAME_LEN);
			}
			else
			{
				wp.name[0] = 'W';
				wp.name[1] = 'P';
				wp.name[2] = (char)('1' + i);
			}
			fieldWaypointSet(i, &wp);
			return i;
		}
	}
	return -1;
}

void fieldWaypointName(const fieldWaypoint_t *wp, char *buf)
{
	memcpy(buf, wp->name, FIELD_WAYPOINT_NAME_LEN);
	buf[FIELD_WAYPOINT_NAME_LEN] = 0;
}

// --- comms schedules --------------------------------------------------------------------------
bool fieldSkedGet(int slot, fieldSked_t *sked)
{
	if ((slot < 0) || (slot >= FIELD_SKED_SLOTS))
	{
		return false;
	}

	SPI_Flash_read((SKEDS_ADDRESS + (slot * sizeof(fieldSked_t))), (uint8_t *)sked, sizeof(fieldSked_t));
	return ((sked->magic == SKED_MAGIC) && skedTimeIsValid(&sked->time));
}

void fieldSkedSet(int slot, const fieldSked_t *sked)
{
	fieldSked_t rec;

	if ((slot < 0) || (slot >= FIELD_SKED_SLOTS))
	{
		return;
	}

	if (sked != NULL)
	{
		rec = *sked;
		rec.magic = SKED_MAGIC;
	}
	else
	{
		memset(&rec, 0xFF, sizeof(rec));
	}
	SPI_Flash_write((SKEDS_ADDRESS + (slot * sizeof(fieldSked_t))), (uint8_t *)&rec, sizeof(rec));
}

// --- coverage survey -----------------------------------------------------------------------
static uint32_t pointAddress(uint32_t index)
{
	return (COVERAGE_ADDRESS + (index * sizeof(coveragePoint_t)));
}

static bool pointIsEmpty(uint32_t index)
{
	uint32_t t = 0U;

	SPI_Flash_read(pointAddress(index), (uint8_t *)&t, sizeof(t));
	return (t == 0xFFFFFFFFU);
}

uint32_t fieldCoverageCount(void)
{
	if (coverageCount == 0xFFFFFFFFU)
	{
		paceStore_t store;

		paceRead(&store);
		if (store.coverageMagic != COVERAGE_MAGIC)
		{
			fieldCoverageClear(); // first use: erase what the GPS log may have left there
			return coverageCount;
		}

		// Points are written in order, so the first empty one is found by bisection
		uint32_t lo = 0U, hi = COVERAGE_POINTS_MAX;

		while (lo < hi)
		{
			uint32_t mid = ((lo + hi) / 2U);

			if (pointIsEmpty(mid))
			{
				hi = mid;
			}
			else
			{
				lo = (mid + 1U);
			}
		}
		coverageCount = lo;
	}
	return coverageCount;
}

bool fieldCoverageAdd(const coveragePoint_t *point)
{
	uint8_t page[PAGE_SIZE];
	uint32_t n = fieldCoverageCount();

	if (n >= COVERAGE_POINTS_MAX)
	{
		return false;
	}

	uint32_t addr = pointAddress(n);
	uint32_t pageAddr = (addr & ~(PAGE_SIZE - 1U));

	// Page program only (the sectors are erased by fieldCoverageClear)
	SPI_Flash_read(pageAddr, page, PAGE_SIZE);
	memcpy(&page[addr - pageAddr], point, sizeof(coveragePoint_t));
	SPI_Flash_writePage(pageAddr, page);
	coverageCount = (n + 1U);
	return true;
}

bool fieldCoverageGet(uint32_t index, coveragePoint_t *point)
{
	if (index >= fieldCoverageCount())
	{
		return false;
	}
	SPI_Flash_read(pointAddress(index), (uint8_t *)point, sizeof(coveragePoint_t));
	return true;
}

void fieldCoverageClear(void)
{
	paceStore_t store;

	for (uint32_t a = COVERAGE_ADDRESS; a < (FIELD_DATA_FLASH_ADDRESS + FIELD_DATA_FLASH_SIZE); a += SECTOR_SIZE)
	{
		SPI_Flash_eraseSector(a);
	}
	coverageCount = 0U;

	paceRead(&store);
	if (store.coverageMagic != COVERAGE_MAGIC)
	{
		store.coverageMagic = COVERAGE_MAGIC;
		storeWrite(&store);
	}
}

#endif
