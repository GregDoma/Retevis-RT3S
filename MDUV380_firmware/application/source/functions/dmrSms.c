/*
 * Fork: DMR SMS receive and inbox (see functions/dmrSms.h), by Marcin SP7TQ.
 * RX path ported from dondch/OpenGD77-AES256 (cleartext only).
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#include <string.h>
#include <stdio.h>
#include "functions/dmrSms.h"

#if defined(HAS_DMR_SMS)

#include "hardware/SPI_Flash.h"
#include "functions/trx.h"
#include "functions/sound.h"
#include "functions/ticks.h"
#include "functions/eventLog.h"
#include "user_interface/uiGlobals.h"
#include "user_interface/uiUtilities.h"
#include "user_interface/menuSystem.h"

#define SECTOR_SIZE         4096U
#define PAGE_SIZE           256U
#define SLOTS_PER_SECTOR    (SECTOR_SIZE / PAGE_SIZE)                  // 16
#define REC_MAGIC           0x314D5344UL                              // "DSM1"
#define EMPTY_WORD          0xFFFFFFFFUL
#define NOTIFICATION_MS     5000U
#define DUP_WINDOW_MS       60000U                                     // a sender's TMS level repeat of the same text

// interfaces/gps.c: LOG_FLASH_16MB_START_ADDRESS + LOG_FLASH_16MB_MEM_SIZE
_Static_assert(((14U * 1024U * 1024U) + (64U * 1024U) + ((2U * 1024U * 1024U) - (128U * 1024U) - (320U * 1024U) - DMR_SMS_FLASH_SIZE)) == DMR_SMS_FLASH_ADDRESS,
		"the GPS log must end where the DMR SMS inbox starts");
_Static_assert((DMR_SMS_FLASH_ADDRESS % SECTOR_SIZE) == 0U, "DMR SMS inbox sector alignment");
_Static_assert((DMR_SMS_SLOTS * PAGE_SIZE) == DMR_SMS_FLASH_SIZE, "DMR SMS inbox size");

typedef struct __attribute__((__packed__))
{
	uint32_t magic;
	uint32_t seq;
	uint32_t time;
	uint32_t src;
	uint32_t dst;
	uint8_t  flags;
	uint8_t  len;
	uint8_t  reserved[2];
} dmrSmsRecordHeader_t;

typedef struct __attribute__((__packed__))
{
	dmrSmsRecordHeader_t h;
	char text[DMR_SMS_TEXT_MAX];
} dmrSmsRecord_t;

_Static_assert(sizeof(dmrSmsRecord_t) <= PAGE_SIZE, "DMR SMS record fits a flash page");

// Interrupt side: reassembly (about 760 bytes, the only large part)
static dmrSmsRx_t rx;

// Inbox index (main loop)
static bool     initialised = false;
static uint8_t  order[DMR_SMS_SLOTS];  // live slots, newest first
static uint8_t  liveCount;
static uint8_t  unreadCount;
static uint8_t  head;                  // next slot to write
static uint32_t nextSeq;
static uint32_t serial;

// Duplicate filter: an unacknowledged confirmed message comes again with the same IP id and
// sequence; the sender's last resort repeat has new ones but the same text
static bool     lastValid;
static uint32_t lastSrc;
static uint16_t lastIpId;
static uint8_t  lastSeq;
static uint8_t  lastFormat;
static uint32_t lastMs;

static uint32_t slotAddress(uint32_t slot)
{
	return (DMR_SMS_FLASH_ADDRESS + (slot * PAGE_SIZE));
}

static bool headerValid(const dmrSmsRecordHeader_t *h)
{
	return ((h->magic == REC_MAGIC) && (h->len <= DMR_SMS_TEXT_MAX));
}

// Reads every slot header: erases sectors that hold neither records nor erased flash (old GPS
// log data), finds the ring head and rebuilds the newest first index of the live messages
static void scan(void)
{
	dmrSmsRecordHeader_t h;
	uint32_t seqs[DMR_SMS_SLOTS];
	uint32_t newestSeq = 0U;
	int newest = -1;

	for (uint32_t sector = 0U; sector < (DMR_SMS_SLOTS / SLOTS_PER_SECTOR); sector++)
	{
		bool anyValid = false, anyDirty = false;

		for (uint32_t i = 0U; i < SLOTS_PER_SECTOR; i++)
		{
			SPI_Flash_read(slotAddress((sector * SLOTS_PER_SECTOR) + i), (uint8_t *)&h, sizeof(h));
			if (headerValid(&h))
			{
				anyValid = true;
			}
			else if (h.magic != EMPTY_WORD)
			{
				anyDirty = true;
			}
		}
		if (anyDirty && (anyValid == false))
		{
			SPI_Flash_eraseSector(slotAddress(sector * SLOTS_PER_SECTOR));
		}
	}

	liveCount = 0U;
	unreadCount = 0U;
	for (uint32_t slot = 0U; slot < DMR_SMS_SLOTS; slot++)
	{
		SPI_Flash_read(slotAddress(slot), (uint8_t *)&h, sizeof(h));
		seqs[slot] = 0U;
		if (headerValid(&h) == false)
		{
			continue;
		}
		if ((newest < 0) || (h.seq > newestSeq))
		{
			newestSeq = h.seq;
			newest = (int)slot;
		}
		if (h.flags & DMR_SMS_FLAG_LIVE)
		{
			// insertion into the newest first index
			uint32_t pos = liveCount;

			seqs[slot] = h.seq;
			while ((pos > 0U) && (seqs[order[pos - 1U]] < h.seq))
			{
				order[pos] = order[pos - 1U];
				pos--;
			}
			order[pos] = (uint8_t)slot;
			liveCount++;
			if (h.flags & DMR_SMS_FLAG_UNREAD)
			{
				unreadCount++;
			}
		}
	}

	if (newest < 0)
	{
		head = 0U;
		nextSeq = 1U;
	}
	else
	{
		head = (uint8_t)((newest + 1) % DMR_SMS_SLOTS);
		nextSeq = (newestSeq + 1U);
	}
	initialised = true;
}

static void ensureInit(void)
{
	if (initialised == false)
	{
		scan();
	}
}

static void store(uint32_t src, uint32_t dst, bool group, const char *text, int len)
{
	uint8_t page[PAGE_SIZE];
	dmrSmsRecord_t *rec = (dmrSmsRecord_t *)page;
	uint32_t addr = slotAddress(head);
	uint32_t word;

	ensureInit();
	SPI_Flash_read(addr, (uint8_t *)&word, sizeof(word));
	if (((head % SLOTS_PER_SECTOR) == 0U) || (word != EMPTY_WORD))
	{
		SPI_Flash_eraseSector(addr & ~(SECTOR_SIZE - 1U)); // the oldest messages were in it
	}

	memset(page, 0xFF, sizeof(page));
	rec->h.magic = REC_MAGIC;
	rec->h.seq = nextSeq++;
	rec->h.time = ((uiDataGlobal.dateTimeSecs > 86400U) ? (uint32_t)uiDataGlobal.dateTimeSecs : 0U);
	rec->h.src = src;
	rec->h.dst = dst;
	rec->h.flags = (uint8_t)(DMR_SMS_FLAG_LIVE | DMR_SMS_FLAG_UNREAD | (group ? DMR_SMS_FLAG_GROUP : 0U) | 0xF8U);
	rec->h.len = (uint8_t)len;
	memcpy(rec->text, text, (size_t)len);
	SPI_Flash_writePage(addr, page);

	head = (uint8_t)((head + 1U) % DMR_SMS_SLOTS);
	scan();
	serial++;
}

// Clears flag bits of a record in place (programming only turns bits to 0)
static void clearFlags(uint32_t slot, uint8_t bits)
{
	uint8_t page[PAGE_SIZE];
	dmrSmsRecord_t *rec = (dmrSmsRecord_t *)page;
	uint32_t addr = slotAddress(slot);

	SPI_Flash_read(addr, page, PAGE_SIZE);
	if (headerValid(&rec->h) && (rec->h.flags & bits))
	{
		rec->h.flags &= (uint8_t)~bits;
		SPI_Flash_writePage(addr, page);
	}
}

uint32_t dmrSmsCount(void)
{
	ensureInit();
	return liveCount;
}

uint32_t dmrSmsUnreadCount(void)
{
	ensureInit();
	return unreadCount;
}

uint32_t dmrSmsSerial(void)
{
	ensureInit();
	return serial;
}

bool dmrSmsGet(uint32_t indexFromNewest, dmrSmsMessage_t *msg)
{
	dmrSmsRecord_t rec;

	ensureInit();
	if (indexFromNewest >= liveCount)
	{
		return false;
	}

	SPI_Flash_read(slotAddress(order[indexFromNewest]), (uint8_t *)&rec, sizeof(rec));
	if (headerValid(&rec.h) == false)
	{
		return false;
	}

	msg->seq = rec.h.seq;
	msg->time = rec.h.time;
	msg->src = rec.h.src;
	msg->dst = rec.h.dst;
	msg->flags = rec.h.flags;
	msg->len = rec.h.len;
	memcpy(msg->text, rec.text, rec.h.len);
	msg->text[rec.h.len] = 0;
	return true;
}

void dmrSmsMarkRead(uint32_t indexFromNewest)
{
	dmrSmsRecordHeader_t h;

	ensureInit();
	if (indexFromNewest >= liveCount)
	{
		return;
	}
	SPI_Flash_read(slotAddress(order[indexFromNewest]), (uint8_t *)&h, sizeof(h));
	if (headerValid(&h) && (h.flags & DMR_SMS_FLAG_UNREAD))
	{
		clearFlags(order[indexFromNewest], DMR_SMS_FLAG_UNREAD);
		if (unreadCount > 0U)
		{
			unreadCount--;
		}
	}
}

void dmrSmsDelete(uint32_t indexFromNewest)
{
	ensureInit();
	if (indexFromNewest < liveCount)
	{
		clearFlags(order[indexFromNewest], DMR_SMS_FLAG_LIVE);
		scan();
		serial++;
	}
}

void dmrSmsDeleteAll(void)
{
	ensureInit();
	for (uint32_t s = 0U; s < (DMR_SMS_SLOTS / SLOTS_PER_SECTOR); s++)
	{
		SPI_Flash_eraseSector(DMR_SMS_FLASH_ADDRESS + (s * SECTOR_SIZE));
	}
	scan();
	serial++;
}

void dmrSmsPeerName(uint32_t id, char *buf, size_t size)
{
	char name[MAX_DMR_ID_CONTACT_TEXT_LENGTH];
	dmrIdDataStruct_t rec;

	id &= 0x00FFFFFFU;
	if (contactIDLookup(id, CONTACT_CALLTYPE_PC, name))
	{
		snprintf(buf, size, "%s", name);
	}
	else if (dmrIDLookup(id, &rec))
	{
		char call[SCREEN_LINE_BUFFER_SIZE];

		getCallsignOnly(call, rec.text);
		snprintf(buf, size, "%s", call);
	}
	else
	{
		snprintf(buf, size, "ID %u", (unsigned)id);
	}
}

// --- interrupt side ----------------------------------------------------------------------
void dmrSmsIsrBurst(int dataType, const uint8_t *payload)
{
	dmrSmsRxFeed(&rx, dataType, payload, trxDMRID);
}

void dmrSmsIsrCallEnd(void)
{
	dmrSmsRxCallEnd(&rx);
}

bool dmrSmsIsrAckTake(uint8_t out[12])
{
	if (rx.ackReady == 0U)
	{
		return false;
	}
	memcpy(out, rx.ack, 12);
	rx.ackReady = 0U;
	return true;
}

// --- main loop ---------------------------------------------------------------------------
static bool isDuplicate(uint32_t src, const dmrSmsPduMeta_t *meta, const char *text, int len)
{
	uint32_t now = ticksGetMillis();
	bool dup = false;

	// a link layer repeat: same sender, format, IP id and sequence (short data has neither)
	if (lastValid && (meta->format != DMR_SMS_FMT_SHORT) && (src == lastSrc) && (meta->format == lastFormat) &&
			(meta->ipId == lastIpId) && (meta->seq == lastSeq))
	{
		return true;
	}

	// the same text from the same sender a moment ago
	if (lastValid && ((now - lastMs) < DUP_WINDOW_MS))
	{
		dmrSmsMessage_t last;

		if (dmrSmsGet(0U, &last) && (last.src == src) && (last.len == len) && (memcmp(last.text, text, (size_t)len) == 0))
		{
			dup = true; // its own link layer repeats are caught by the rule above from now on
		}
	}

	lastValid = true;
	lastSrc = src;
	lastIpId = meta->ipId;
	lastSeq = meta->seq;
	lastFormat = meta->format;
	if (dup == false)
	{
		lastMs = now;
	}
	return dup;
}

void dmrSmsTick(void)
{
	char text[DMR_SMS_TEXT_MAX + 1];
	dmrSmsPduMeta_t meta;
	dmrSmsPduInfo_t info;
	int len;

	if (rx.ready == 0U)
	{
		return;
	}

	info = rx.info;
	len = dmrSmsPduDecode(rx.pdu, &info, text, sizeof(text), &meta);
	rx.ready = 0U; // pdu[] back to the interrupt

	if ((len <= 0) || (dmrSmsPduForUs(&info, trxDMRID) == false) || isDuplicate(info.src, &meta, text, len))
	{
		return;
	}

	store(info.src, info.dst, (info.group != 0U), text, len);

	char peer[SCREEN_LINE_BUFFER_SIZE];
	char note[48];

	dmrSmsPeerName(info.src, peer, sizeof(peer));
	snprintf(note, sizeof(note), "SMS %s:%s", peer, text);
	uiNotificationShow(NOTIFICATION_TYPE_MESSAGE, NOTIFICATION_ID_MESSAGE, NOTIFICATION_MS, note, false);
#if defined(HAS_EVENT_LOG)
	eventLogAdd(EVENT_LOG_MSG_IN, "%u:%s", (unsigned)info.src, text);
#endif
	soundSetMelody(MELODY_PRIVATE_CALL);
}

#endif
