/*
 * Fork: comms schedule ("sked") timing, by Marcin SP7TQ.
 *
 * Hardware independent (host tested in tests/aprs_rx/test_sked.c). A sked repeats every
 * 'periodMin' minutes from 'offsetMin' minutes after 00:00 UTC; the period divides a day
 * (15 / 20 / 30 min, 1 / 2 / 3 / 4 / 6 / 8 / 12 h, daily), so every day has the same start
 * times and midnight / the hour need no special case:
 *   every hour at :00         period 60,   offset 0
 *   every 30 min from :15     period 30,   offset 15
 *   daily at 18:00 UTC        period 1440, offset 1080
 * A sked is "on" for 'durationMin' minutes from each start.
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#ifndef _OPENGD77_SKED_MATH_H_
#define _OPENGD77_SKED_MATH_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define SKED_DAY_MIN          1440U
#define SKED_TUNE_NONE        0U  // no channel change offered
#define SKED_TUNE_PACE_FIRST  1U  // 1..4: PACE slot P / A / C / E
#define SKED_TUNE_CHANNEL     5U  // the channel stored in the sked
#define SKED_TUNE_MAX         6U

typedef struct
{
	uint16_t periodMin;   // divides 1440
	uint16_t offsetMin;   // 0 .. periodMin - 1
	uint8_t  durationMin; // 1 .. periodMin
	uint8_t  tune;        // SKED_TUNE_*
	uint8_t  enabled;     // 1: on, 0: kept but not alerted
	uint8_t  reserved;
} skedTime_t;

bool skedTimeIsValid(const skedTime_t *s);
void skedTimeDefault(skedTime_t *s); // every hour at :00, 10 min, no tune, on

// Seconds from 'utc' to the next start (0: a start is at 'utc'), and since the last one
uint32_t skedSecondsToStart(uint32_t utc, const skedTime_t *s);
uint32_t skedSecondsSinceStart(uint32_t utc, const skedTime_t *s);
bool skedIsOn(uint32_t utc, const skedTime_t *s); // within 'durationMin' of a start

// Editing steps (dir +1 / -1), wrapping
uint16_t skedPeriodStep(uint16_t periodMin, int dir);
uint16_t skedOffsetStep(uint16_t periodMin, uint16_t offsetMin, int dir); // 5 min (periods up to 1 h), else 15 min
uint8_t  skedDurationStep(uint16_t periodMin, uint8_t durationMin, int dir);
void     skedTimeFix(skedTime_t *s); // offset / duration back in range after a period change

// "HOURLY :15", "EVERY 30M :15", "EVERY 2H 01:00", "DAILY 18:00"
void skedFormatWhen(const skedTime_t *s, char *buf, size_t size);
// Count-down: "mm:ss" under 100 min, else "hHmm" (e.g. "5H12")
void skedFormatCountdown(uint32_t secs, char *buf, size_t size);

#endif
