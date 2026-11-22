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

// A value the user changed during the blackout (display options) is kept as the user value
static void adoptUserChanges(void)
{
	blackoutBacklight_t now, dark;

	readSettings(&now);
	blackoutValues(&dark);
	if (now.backlightMode != dark.backlightMode) { userValues.backlightMode = now.backlightMode; }
	if (now.backLightTimeout != dark.backLightTimeout) { userValues.backLightTimeout = now.backLightTimeout; }
	if (now.percentage[DAY] != dark.percentage[DAY]) { userValues.percentage[DAY] = now.percentage[DAY]; }
	if (now.percentage[NIGHT] != dark.percentage[NIGHT]) { userValues.percentage[NIGHT] = now.percentage[NIGHT]; }
	if (now.percentageOff != dark.percentageOff) { userValues.percentageOff = now.percentageOff; }
}

void blackoutSettingsForSave(bool saving)
{
	if (active == false)
	{
		return;
	}

	if (saving)
	{
		adoptUserChanges();
		writeSettings(&userValues); // the flash gets the user values, never the blackout ones
	}
	else
	{
		blackoutBacklight_t dark;

		blackoutValues(&dark);
		writeSettings(&dark);
	}
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
		adoptUserChanges();
		writeSettings(&userValues);
		active = false;

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
