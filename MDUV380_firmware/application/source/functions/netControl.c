/*
 * Fork: EMCOM net control, by Marcin SP7TQ. See netControl.h.
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#include <string.h>
#include <stdio.h>
#include "functions/netControl.h"

#if defined(HAS_NET_CONTROL)

#include "user_interface/menuSystem.h"
#include "user_interface/uiUtilities.h"
#include "functions/codeplug.h"
#include "functions/sound.h"
#include "functions/ticks.h"
#include "functions/trx.h"
#if defined(I2S_APRS_RX)
#include "functions/aprsRx.h"
#include "user_interface/uiLocalisation.h" // Fork (M0SQQ): TR()
#endif

#define TICK_MS  1000U

static const uint8_t quietLimitsMin[] = { 10U, 15U, 30U, 5U }; // [0]: the default (state in .bss), '#' steps 10 > 15 > 30 > 5

typedef struct
{
	netRoster_t roster;
	uint32_t    startMs;
	uint32_t    startUtc;
	uint32_t    serial;
	bool        running;
	uint8_t     quietIndex; // quietLimitsMin[] (RAM only: no spare settings bit)
} netState_t;

static netState_t net;

bool netIsRunning(void)
{
	return net.running;
}

const netRoster_t *netGetRoster(void)
{
	return &net.roster;
}

uint16_t netNowS(void)
{
	return netSeconds(ticksGetMillis() - net.startMs);
}

uint32_t netStartUtc(void)
{
	return net.startUtc;
}

uint16_t netQuietLimitS(void)
{
	return (uint16_t)(quietLimitsMin[net.quietIndex] * 60U);
}

void netQuietLimitCycle(void)
{
	net.quietIndex = (uint8_t)((net.quietIndex + 1U) % (sizeof(quietLimitsMin) / sizeof(quietLimitsMin[0])));
	net.serial++;
}

uint32_t netSerial(void)
{
	return net.serial;
}

void netChannelName(char *buf, size_t size)
{
	if (menuSystemGetRootMenuNumber() == UI_CHANNEL_MODE)
	{
		char name[20];

		codeplugUtilConvertBufToString(currentChannelData->name, name, 16);
		snprintf(buf, size, "%s", name);
	}
	else
	{
		uint32_t f = currentChannelData->rxFreq;

		snprintf(buf, size, "%u.%04u", (unsigned)(f / 100000U), (unsigned)((f % 100000U) / 10U));
	}
}

void netStart(void)
{
	char name[20];

	netRosterInit(&net.roster);
	net.startMs = ticksGetMillis();
	net.startUtc = ((uiDataGlobal.dateTimeSecs > 86400U) ? uiDataGlobal.dateTimeSecs : 0U);
	net.running = true;
	net.serial++;

	netChannelName(name, sizeof(name));
	name[EVENT_LOG_TEXT_LEN - 1U - 6U] = 0; // "START " + name fits the record
	if (name[0])
	{
		eventLogAdd(EVENT_LOG_NET, "START %s", name);
	}
	else
	{
		eventLogAdd(EVENT_LOG_NET, "START");
	}
}

void netEnd(void)
{
	if (net.running)
	{
		net.running = false;
		net.serial++;
		eventLogAdd(EVENT_LOG_NET, "END %u", (unsigned)net.roster.count);
	}
}

static void logIn(int index)
{
	char call[NET_CALL_BUF];

	netEntryCall(&net.roster.e[index], call);
	eventLogAdd(EVENT_LOG_NET, "IN %s", call);
}

int netAddFm(void)
{
	int index = (net.running ? netRosterAddFm(&net.roster, netNowS()) : -1);

	if (index >= 0)
	{
		net.serial++;
		logIn(index);
	}
	return index;
}

void netMarkHeard(int index)
{
	if (net.running && (index >= 0) && (index < (int)net.roster.count))
	{
		netRosterHeard(&net.roster, net.roster.e[index].key, NULL, netNowS(), NULL);
		net.serial++;
	}
}

void netLogQso(const char *station)
{
	char text[EVENT_LOG_TEXT_LEN];

	netQsoText(text, sizeof(text), station, "59", NULL);
	eventLogAdd(EVENT_LOG_QSO, "%s", text);
	soundSetMelody(MELODY_ACK_BEEP);
	uiNotificationShowKind(UI_ALERT_INFO, NOTIFICATION_TYPE_MESSAGE, NOTIFICATION_ID_MESSAGE, 1500U, TR("QSO logged"), true);
}

void netStationName(const LinkItem_t *item, char *buf, size_t size)
{
	char call[SCREEN_LINE_BUFFER_SIZE];
	const char *src = (((item->talkerAlias[0] != 0) && (strncmp(item->contact, "ID:", 3) == 0)) ? item->talkerAlias : item->contact);

	getCallsignOnly(call, src);
	if ((call[0] == 0) || (strncmp(call, "ID:", 3) == 0))
	{
		snprintf(buf, size, "%u", (unsigned)(item->id & 0xFFFFFFU));
	}
	else
	{
		snprintf(buf, size, "%s", call);
	}
}

static void heard(uint32_t key, const char *call, uint32_t timeMs)
{
	int32_t sinceStart = (int32_t)(timeMs - net.startMs);
	int index;

	if (sinceStart < 0)
	{
		return; // heard before the net started
	}

	uint16_t atS = netSeconds((uint32_t)sinceStart);
	int before = netRosterFind(&net.roster, key);
	uint16_t lastS = ((before >= 0) ? net.roster.e[before].lastS : 0U);

	if ((before >= 0) && (atS <= lastS))
	{
		return; // nothing new
	}

	if (netRosterHeard(&net.roster, key, call, atS, &index) == NET_HEARD_NEW)
	{
		logIn(index);
	}
	net.serial++;
}

// Screens that own the display: no card over them (the alarm is still logged)
static bool cardAllowed(void)
{
	int menu = menuSystemGetCurrentMenuNumber();

	return ((menu != UI_CPS) && (menu != UI_HOTSPOT_MODE) && (menu != UI_POWER_OFF) && (menu != UI_SPLASH_SCREEN) &&
			(menu != MENU_CALIBRATION) && (menu != UI_TX_SCREEN));
}

void netTick(void)
{
	static uint32_t lastTick = 0U;
	uint32_t now = ticksGetMillis();
	char call[SCREEN_LINE_BUFFER_SIZE];

	if ((net.running == false) || ((now - lastTick) < TICK_MS))
	{
		return;
	}
	lastTick = now;

	// DMR: the last heard list
	if (uiDataGlobal.lastHeardCount > 0)
	{
		for (LinkItem_t *item = LinkHead; (item != NULL) && (item->id != 0U); item = item->next)
		{
			netStationName(item, call, sizeof(call));
			heard(netDmrKey(item->id), call, item->time);
		}
	}

#if defined(I2S_APRS_RX)
	// APRS: the stations heard (not objects / items, nor my own beacons coming back through a digipeater)
	{
		char own[APRS_RX_CALL_LEN + 1];
		bool hasOwn = aprsRxGetOwnCall(own, sizeof(own));
		uint32_t n = aprsRxStationCount();

		for (uint32_t i = 0U; i < n; i++)
		{
			const aprsRxStation_t *st = aprsRxStationGet(i);

			if ((st != NULL) && st->name[0] && (st->isObject == false) && ((hasOwn == false) || (strcmp(st->name, own) != 0)))
			{
				heard(netAprsKey(st->name), st->name, st->lastMs);
			}
		}
	}
#endif

	// silence alarm, one station per tick, not while transmitting
	if (trxTransmissionEnabled == false)
	{
		int index = netRosterQuietNext(&net.roster, netNowS(), netQuietLimitS());

		if (index >= 0)
		{
			char msg[NET_CALL_BUF + 8];

			netEntryCall(&net.roster.e[index], call);
			eventLogAdd(EVENT_LOG_NET, "QUIET %s", call);
			net.serial++;
			if (cardAllowed())
			{
				snprintf(msg, sizeof(msg), TR("QUIET %s"), call);
				soundSetMelody(MELODY_NACK_BEEP);
				uiNotificationShowKind(UI_ALERT_WARNING, NOTIFICATION_TYPE_MESSAGE, NOTIFICATION_ID_MESSAGE, 3000U, msg, true);
			}
		}
	}
}

#endif // HAS_NET_CONTROL
