/*
 * Fork: comms schedule reminders, by Marcin SP7TQ. See sked.h.
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#include <string.h>
#include <stdio.h>
#include "functions/sked.h"

#if defined(HAS_SKED)

#include "user_interface/menuSystem.h"
#include "user_interface/uiUtilities.h"
#include "functions/blackout.h"
#include "functions/codeplug.h"
#include "functions/eventLog.h"
#include "functions/settings.h"
#include "functions/sound.h"
#include "functions/trx.h"

#define CLOCK_VALID_SECS   1704067200U // 2024-01-01: earlier means the clock was never set
#define CARD_WARN_MS       5000U
#define CARD_NOW_MS        20000U

static const char paceLetters[PACE_SLOTS] = { 'P', 'A', 'C', 'E' };

typedef struct
{
	uint32_t lastMinute; // UTC minute checked last, 0: none yet
	int8_t   tuneSlot;   // sked whose "NOW" card offers GREEN = tune, -1: none
} skedRuntime_t;

static skedRuntime_t rt = { 0U, -1 };

bool skedClockValid(void)
{
	return (uiDataGlobal.dateTimeSecs >= CLOCK_VALID_SECS);
}

void skedNextGet(skedNext_t *next)
{
	fieldSked_t s;
	uint32_t now = uiDataGlobal.dateTimeSecs;

	next->slot = -1;
	next->secs = 0U;
	next->on = false;

	if (skedClockValid() == false)
	{
		return;
	}

	for (int i = 0; i < FIELD_SKED_SLOTS; i++)
	{
		if (fieldSkedGet(i, &s) && s.time.enabled)
		{
			if (skedIsOn(now, &s.time))
			{
				uint32_t since = skedSecondsSinceStart(now, &s.time);

				if ((next->on == false) || (since < next->secs))
				{
					next->slot = i;
					next->secs = since;
					next->on = true;
				}
			}
			else if (next->on == false)
			{
				uint32_t to = skedSecondsToStart(now, &s.time);

				if ((next->slot < 0) || (to < next->secs))
				{
					next->slot = i;
					next->secs = to;
				}
			}
		}
	}
}

static bool tuneChannel(const fieldSked_t *s, paceSlot_t *ch)
{
	if ((s->time.tune >= SKED_TUNE_PACE_FIRST) && (s->time.tune < (SKED_TUNE_PACE_FIRST + PACE_SLOTS)))
	{
		return fieldPaceGet((s->time.tune - SKED_TUNE_PACE_FIRST), ch);
	}

	if (s->time.tune == SKED_TUNE_CHANNEL)
	{
		*ch = s->channel;
		return ((ch->zoneNumber >= 0) && (ch->channelNumber > 0) && (ch->channelNumber <= 1024));
	}
	return false;
}

void skedTargetName(const fieldSked_t *s, char *buf, size_t size)
{
	paceSlot_t ch;
	char name[20];

	if (tuneChannel(s, &ch) && codeplugAllChannelsIndexIsInUse(ch.channelNumber))
	{
		CodeplugChannel_t data;

		codeplugChannelGetDataForIndex(ch.channelNumber, &data);
		codeplugUtilConvertBufToString(data.name, name, 16);
		snprintf(buf, size, "%s", name);
		return;
	}

	if ((s->time.tune >= SKED_TUNE_PACE_FIRST) && (s->time.tune < (SKED_TUNE_PACE_FIRST + PACE_SLOTS)))
	{
		snprintf(buf, size, "PACE %c (empty)", paceLetters[s->time.tune - SKED_TUNE_PACE_FIRST]);
	}
	else if (menuSystemGetRootMenuNumber() == UI_CHANNEL_MODE)
	{
		codeplugUtilConvertBufToString(currentChannelData->name, name, 16);
		snprintf(buf, size, "%s", name);
	}
	else
	{
		uint32_t f = currentChannelData->rxFreq;

		snprintf(buf, size, "%u.%04u", (unsigned)(f / 100000U), (unsigned)((f % 100000U) / 10U));
	}
}

bool skedTuneTo(const fieldSked_t *s)
{
	paceSlot_t ch;

	return (tuneChannel(s, &ch) && paceActivateChannel(&ch));
}

static bool onMainScreen(void)
{
	int menu = menuSystemGetCurrentMenuNumber();

	return ((menu == UI_CHANNEL_MODE) || (menu == UI_VFO_MODE));
}

// Screens that own the display: no card over them (the start is still logged)
static bool cardAllowed(void)
{
	int menu = menuSystemGetCurrentMenuNumber();

	return ((trxTransmissionEnabled == false) && (menu != UI_CPS) && (menu != UI_HOTSPOT_MODE) && (menu != UI_POWER_OFF) &&
			(menu != UI_SPLASH_SCREEN) && (menu != MENU_CALIBRATION) && (menu != UI_TX_SCREEN));
}

static void alert(const int16_t *melody)
{
	if (blackoutActive() == false)
	{
		soundSetMelody(melody);
	}
}

void skedTick(void)
{
	fieldSked_t s;
	char msg[NOTIFICATION_MESSAGE_LEN_MAX];
	char name[24];

	if (skedClockValid() == false)
	{
		return;
	}

	uint32_t minute = (uiDataGlobal.dateTimeSecs / 60U);

	if (minute == rt.lastMinute)
	{
		return; // starts and warnings fall on whole minutes: one check per minute
	}

	bool first = (rt.lastMinute == 0U);
	rt.lastMinute = minute;
	if (first)
	{
		return; // booted / clock set in the middle of a minute
	}

	uint32_t t = (minute * 60U);

	for (int i = 0; i < FIELD_SKED_SLOTS; i++)
	{
		if ((fieldSkedGet(i, &s) == false) || (s.time.enabled == 0U))
		{
			continue;
		}

		uint32_t to = skedSecondsToStart(t, &s.time);

		if (to == 60U)
		{
			if (cardAllowed())
			{
				skedTargetName(&s, name, sizeof(name));
				snprintf(msg, sizeof(msg), "SKED in 1 min\nS%d %s", (i + 1), name);
				alert(MELODY_NACK_BEEP);
				uiNotificationShowKind(UI_ALERT_WARNING, NOTIFICATION_TYPE_MESSAGE, NOTIFICATION_ID_MESSAGE, CARD_WARN_MS, msg, true);
			}
		}
		else if (to == 0U)
		{
			paceSlot_t ch;
			bool canTune = (tuneChannel(&s, &ch) && codeplugAllChannelsIndexIsInUse(ch.channelNumber));

#if defined(HAS_EVENT_LOG)
			eventLogAdd(EVENT_LOG_SYS, "SKED %d", (i + 1));
#endif
			if (cardAllowed())
			{
				skedTargetName(&s, name, sizeof(name));
				snprintf(msg, sizeof(msg), "SKED %d NOW\n%s%s", (i + 1), name, ((canTune && onMainScreen()) ? "\nGREEN = TUNE" : ""));
				rt.tuneSlot = (int8_t)(canTune ? i : -1);
				alert(MELODY_ACK_BEEP);
				uiNotificationShowKind(UI_ALERT_INFO, NOTIFICATION_TYPE_MESSAGE, NOTIFICATION_ID_USER_SKED, CARD_NOW_MS, msg, true);
			}
		}
	}
}

bool skedHandleKey(uiEvent_t *ev)
{
	fieldSked_t s;

	if ((rt.tuneSlot < 0) || ((ev->events & KEY_EVENT) == 0) || (uiNotificationIsVisible() == false) ||
			(uiNotificationGetId() != NOTIFICATION_ID_USER_SKED))
	{
		return false;
	}

	if ((KEYCHECK_SHORTUP(ev->keys, KEY_GREEN) == false) || (onMainScreen() == false) ||
			BUTTONCHECK_DOWN(ev, BUTTON_SK1) || BUTTONCHECK_DOWN(ev, BUTTON_SK2))
	{
		return false;
	}

	int slot = rt.tuneSlot;

	rt.tuneSlot = -1;
	uiNotificationHide(false);
	if (fieldSkedGet(slot, &s) && skedTuneTo(&s))
	{
		soundSetMelody(MELODY_ACK_BEEP);
	}
	else
	{
		soundSetMelody(MELODY_ERROR_BEEP);
	}
	return true;
}

#endif // HAS_SKED
