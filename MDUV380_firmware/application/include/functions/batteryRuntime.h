/*
 * Fork: battery runtime estimate ("time left"), by Marcin SP7TQ.
 *
 * Hardware independent (host tested in tests/aprs_rx/test_batt.c). The firmware feeds the
 * averaged battery voltage (0.01 V units) with the time; samples taken while transmitting
 * (and just after) are flagged and ignored. The voltage is kept per 150 s slot as the
 * highest value seen in it (the rest voltage: TX / speaker sag and noise only pull the
 * voltage down), the drain rate is the least squares slope over the last 30 minutes of
 * slots, smoothed, and the remaining charge is mapped through the same linear voltage
 * curve as the battery percentage. The minutes shown count down by themselves and are
 * pulled slowly towards each new estimate, so they do not jump.
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#ifndef _OPENGD77_BATTERY_RUNTIME_H_
#define _OPENGD77_BATTERY_RUNTIME_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define BATT_RT_SLOTS          12U          // 12 x 150 s = 30 min
#define BATT_RT_SLOT_MS        150000U
#define BATT_RT_MIN_SLOTS      6            // 15 min of data before an estimate
#define BATT_RT_MAX_MINUTES    (99 * 60)
#define BATT_RT_UNKNOWN        (-1)
#define BATT_RT_CHARGE_JUMP    8            // 0.08 V higher than the previous slot: charger plugged in

typedef struct
{
	int16_t  v[BATT_RT_SLOTS];   // slot rest voltage, 0.01 V
	uint8_t  seq[BATT_RT_SLOTS]; // slot number (wraps), for slots without samples
	uint32_t slotStartMs;
	float    rate;               // smoothed drain, 0.01 V per slot (> 0: discharging)
	float    shownMin;           // minutes shown, < 0: unknown
	int16_t  slotMax;            // current slot, -1: no sample yet
	int16_t  vEmpty;             // 0.01 V at 0 %
	uint8_t  slotSeq;
	uint8_t  head;
	uint8_t  count;
	bool     started;
} batteryRuntime_t;              // 56 bytes

void batteryRuntimeInit(batteryRuntime_t *b, int16_t vEmpty);
// nowMs: a millisecond clock, v: voltage in 0.01 V, exclude: TX now or just before
void batteryRuntimeAddSample(batteryRuntime_t *b, uint32_t nowMs, int16_t v, bool exclude);
int batteryRuntimeMinutes(const batteryRuntime_t *b);       // BATT_RT_UNKNOWN or 0..BATT_RT_MAX_MINUTES
// "~3h20", "~45m", "~12h", "--" (buffer >= 7)
void batteryRuntimeFormat(int minutes, char *buf, size_t size);

#endif
