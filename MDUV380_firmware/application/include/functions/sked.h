/*
 * Fork: comms schedule ("sked") reminders, by Marcin SP7TQ.
 *
 * Up to 4 skeds in the field data flash (fieldSked_t, timing in functions/skedMath.h),
 * edited on the Sked screen (PACE > '#', quickkey "Sked"). skedTick() (main loop) works
 * on the UTC clock, once per minute: 1 min before a start a "SKED in 1 min" warning
 * card and a beep, at the start a "SKED NOW <channel>" card, a beep and "SKED <n>" in
 * the event log (SYS). When the sked has a PACE slot / channel and the channel / VFO
 * screen is shown, GREEN on that card switches to it (skedHandleKey()); the radio never
 * changes channel by itself. In a blackout the cards are shown without a beep.
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#ifndef _OPENGD77_SKED_H_
#define _OPENGD77_SKED_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "functions/fieldData.h"

#if defined(HAS_FIELD_TOOLS)
#define HAS_SKED

#include "user_interface/menuSystem.h"

typedef struct
{
	int      slot;  // -1: no sked (or no clock)
	uint32_t secs;  // to the next start, or since the start when 'on'
	bool     on;    // within its duration now
} skedNext_t;

bool skedClockValid(void);                // the UTC clock is set
void skedNextGet(skedNext_t *next);       // the sked on now, else the next to start
void skedTargetName(const fieldSked_t *s, char *buf, size_t size); // "PRIMARY CH 1", "CH name", the current channel
bool skedTuneTo(const fieldSked_t *s);    // PACE slot / stored channel (channel mode), false: nothing to tune
void skedTick(void);                      // main loop
bool skedHandleKey(uiEvent_t *ev);        // GREEN on the "SKED NOW" card: tunes, the key is consumed (true)

#endif
#endif
