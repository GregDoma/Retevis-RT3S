/*
 * Fork: blackout / silent mode, by Marcin SP7TQ. See blackout.h.
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#include "functions/blackout.h"
#include "functions/settings.h"
#include "functions/sound.h"
#include "functions/voicePrompts.h"
#include "io/display.h"
#include "io/LEDs.h"
#include "user_interface/menuSystem.h"
#include "user_interface/uiGlobals.h"

typedef struct
{
	uint8_t backlightMode;
	uint8_t backLightTimeout;
	int8_t  percentage[NIGHT + 1];
	int8_t  percentageOff;
} blackoutBacklight_t;

static blackoutBacklight_t userValues;  // the user settings while active
static bool active = false;

bool blackoutActive(void)
{
	return active;
}

static void blackoutValues(blackoutBacklight_t *b)
{
	b->backlightMode = BACKLIGHT_MODE_BUTTONS; // off until a key press, nothing else lights it
	b->backLightTimeout = ((userValues.backLightTimeout != 0U) ? userValues.backLightTimeout : BLACKOUT_BACKLIGHT_TIMEOUT);
	b->percentage[DAY] = (int8_t)BLACKOUT_BACKLIGHT_PERCENT;
	b->percentage[NIGHT] = (int8_t)BLACKOUT_BACKLIGHT_PERCENT;
	b->percentageOff = 0;
}

static void readSettings(blackoutBacklight_t *b)
{
	b->backlightMode = nonVolatileSettings.backlightMode;
	b->backLightTimeout = nonVolatileSettings.backLightTimeout;
	b->percentage[DAY] = nonVolatileSettings.displayBacklightPercentage[DAY];
	b->percentage[NIGHT] = nonVolatileSettings.displayBacklightPercentage[NIGHT];
	b->percentageOff = nonVolatileSettings.displayBacklightPercentageOff;
}

static void writeSettings(const blackoutBacklight_t *b)
{
	nonVolatileSettings.backlightMode = b->backlightMode;
	nonVolatileSettings.backLightTimeout = b->backLightTimeout;
	nonVolatileSettings.displayBacklightPercentage[DAY] = b->percentage[DAY];
	nonVolatileSettings.displayBacklightPercentage[NIGHT] = b->percentage[NIGHT];
	nonVolatileSettings.displayBacklightPercentageOff = b->percentageOff;
}

// Display options saved during the blackout: a backlight value the user changed there
// (compared with the values when the menu was opened, not with the blackout values, so
// picking e.g. 5 % or "Buttons" is kept too) becomes the user value.
void blackoutAdoptBacklightEdits(const settingsStruct_t *before)
{
	if (active == false)
	{
		return;
	}

	if (nonVolatileSettings.backlightMode != before->backlightMode) { userValues.backlightMode = nonVolatileSettings.backlightMode; }
	if (nonVolatileSettings.backLightTimeout != before->backLightTimeout) { userValues.backLightTimeout = nonVolatileSettings.backLightTimeout; }
	if (nonVolatileSettings.displayBacklightPercentage[DAY] != before->displayBacklightPercentage[DAY]) { userValues.percentage[DAY] = nonVolatileSettings.displayBacklightPercentage[DAY]; }
	if (nonVolatileSettings.displayBacklightPercentage[NIGHT] != before->displayBacklightPercentage[NIGHT]) { userValues.percentage[NIGHT] = nonVolatileSettings.displayBacklightPercentage[NIGHT]; }
	if (nonVolatileSettings.displayBacklightPercentageOff != before->displayBacklightPercentageOff) { userValues.percentageOff = nonVolatileSettings.displayBacklightPercentageOff; }
}

// Settings reset to the defaults during the blackout: the defaults are the user values now
void blackoutSettingsWereReset(void)
{
	if (active)
	{
		blackoutBacklight_t dark;

		readSettings(&userValues);
		blackoutValues(&dark);
		writeSettings(&dark);
	}
}

// The copy of the settings written to the flash gets the user values (the live ones stay dark)
void blackoutSettingsForSave(settingsStruct_t *copy)
{
	copy->backlightMode = userValues.backlightMode;
	copy->backLightTimeout = userValues.backLightTimeout;
	copy->displayBacklightPercentage[DAY] = userValues.percentage[DAY];
	copy->displayBacklightPercentage[NIGHT] = userValues.percentage[NIGHT];
	copy->displayBacklightPercentageOff = userValues.percentageOff;
}

void blackoutToggle(void)
{
	if (active == false)
	{
		blackoutBacklight_t dark;

		readSettings(&userValues);
		blackoutValues(&dark);
		writeSettings(&dark);

		soundStopMelody();
		voicePromptsTerminate();
		active = true;

#if ! defined(PLATFORM_GD77S)
		// LEDs off; LEDsState still follows RX / TX (the EMCOM screen reads it)
		for (int led = 0; led < NUM_LEDS; led++)
		{
			uint8_t state = LEDsState[led];

			LedWriteDirect((LEDs_t)led, 0);
			LEDsState[led] = state;
		}
#endif
		displayLightTrigger(true); // the minimum level for the timeout, then off
	}
	else
	{
		writeSettings(&userValues);
		active = false;
		voicePromptsInit(); // drop anything queued while silent, so no stale prompt plays

#if ! defined(PLATFORM_GD77S)
		if (settingsIsOptionBitSet(BIT_ALL_LEDS_DISABLED) == false)
		{
			for (int led = 0; led < NUM_LEDS; led++)
			{
				LedWriteDirect((LEDs_t)led, LEDsState[led]);
			}
		}
#endif
		if (nonVolatileSettings.backlightMode == BACKLIGHT_MODE_NONE)
		{
			displayEnableBacklight(false, nonVolatileSettings.displayBacklightPercentageOff);
		}
		else if (nonVolatileSettings.backlightMode == BACKLIGHT_MODE_MANUAL)
		{
			displayEnableBacklight(true, nonVolatileSettings.displayBacklightPercentageOff);
		}
		else
		{
			displayLightTrigger(true);
		}
	}

	uiNotificationShowKind(UI_ALERT_INFO, NOTIFICATION_TYPE_MESSAGE, NOTIFICATION_ID_MESSAGE, 1500U, (active ? "BLACKOUT ON" : "BLACKOUT OFF"), false);
}
