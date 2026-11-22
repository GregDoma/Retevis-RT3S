/*
 * Fork: blackout / silent mode, by Marcin SP7TQ.
 *
 * One toggle (SK1 + 5 on the channel / VFO screen, or the "Blackout" quickkey action) for
 * night or covert work: the backlight stays off until a key press and then lights at a
 * minimum level for the normal timeout, no beeps / key tones / voice prompts, no RX / TX
 * LED. TX, the receive audio, SOS beacons and visual alerts are not touched.
 *
 * The backlight settings are swapped for the blackout values in RAM and back on exit;
 * when the settings are written to the flash (power off...) the user values are written,
 * so the radio always boots normal. A backlight setting changed by the user during the
 * blackout is kept. Beeps, voice prompts and LEDs are gated by blackoutActive().
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#ifndef _OPENGD77_BLACKOUT_H_
#define _OPENGD77_BLACKOUT_H_

#include <stdbool.h>

#define BLACKOUT_BACKLIGHT_PERCENT   5U  // lit level after a key press
#define BLACKOUT_BACKLIGHT_TIMEOUT   10U // seconds, when the user timeout is "never"

bool blackoutActive(void);
void blackoutToggle(void);          // shows the "BLACKOUT ON / OFF" card
void blackoutSettingsForSave(bool saving); // settingsSaveSettings(): true before the write (user values), false after

#endif
