/*
 * Fork: battery runtime estimate ("time left"), by Marcin SP7TQ. See batteryRuntime.h.
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#include <stdio.h>
#include "functions/batteryRuntime.h"

#define SLOT_MINUTES        (BATT_RT_SLOT_MS / 60000.0f)
#define CHARGE_DRAIN        (-0.5f)    // rising by more than 0.005 V per slot: charging
#define UNPLUG_DROP         10         // 0.10 V lower than the previous slot: charger removed / new load
#define GAP_SLOTS           4U         // longer without samples (suspend): start again
#define RATE_SMOOTHING      0.25f
#define SHOWN_SMOOTHING     0.25f

static void restart(batteryRuntime_t *b)
{
	b->count = 0U;
	b->rate = 0.0f;
	b->shownMin = -1.0f;
}

void batteryRuntimeInit(batteryRuntime_t *b, int16_t vEmpty)
{
	b->head = 0U;
	b->slotSeq = 0U;
	b->slotMax = -1;
	b->started = false;
	b->vEmpty = vEmpty;
	restart(b);
}

static float estimateMinutes(batteryRuntime_t *b)
{
	float n = 0.0f, sx = 0.0f, sy = 0.0f, sxx = 0.0f, sxy = 0.0f;
	uint8_t newest = b->seq[(b->head + BATT_RT_SLOTS - 1U) % BATT_RT_SLOTS];

	for (uint8_t i = 0U; i < b->count; i++)
	{
		uint8_t k = (uint8_t)((b->head + BATT_RT_SLOTS - 1U - i) % BATT_RT_SLOTS);
		float x = -(float)((uint8_t)(newest - b->seq[k]));
		float y = (float)(b->v[k] - b->vEmpty);

		n += 1.0f; sx += x; sy += y; sxx += (x * x); sxy += (x * y);
	}

	float den = ((n * sxx) - (sx * sx));
	if (den <= 0.0f)
	{
		return -1.0f;
	}

	float slope = (((n * sxy) - (sx * sy)) / den);  // 0.01 V per slot
	float left = ((sy - (slope * sx)) / n);         // fitted at the newest slot, 0.01 V above empty
	float drain = -slope;

	if (drain < CHARGE_DRAIN)
	{
		b->rate = 0.0f;
		return -1.0f; // charging / recovering: unknown
	}

	b->rate = ((b->rate == 0.0f) ? drain : (b->rate + ((drain - b->rate) * RATE_SMOOTHING)));

	if (left <= 0.0f)
	{
		return 0.0f;
	}
	if ((b->rate * BATT_RT_MAX_MINUTES) <= (left * SLOT_MINUTES))
	{
		return (float)BATT_RT_MAX_MINUTES; // flat (or within the noise): clamp
	}
	return ((left / b->rate) * SLOT_MINUTES);
}

static void closeSlot(batteryRuntime_t *b, uint32_t slots)
{
	// the minutes shown count down by themselves
	if (b->shownMin >= 0.0f)
	{
		b->shownMin -= (SLOT_MINUTES * (float)slots);
		if (b->shownMin < 0.0f)
		{
			b->shownMin = 0.0f;
		}
	}

	if (b->slotMax < 0)
	{
		return; // all TX in this slot
	}

	if (b->count > 0U)
	{
		int16_t last = b->v[(b->head + BATT_RT_SLOTS - 1U) % BATT_RT_SLOTS];

		if ((b->slotMax >= (last + BATT_RT_CHARGE_JUMP)) || (b->slotMax <= (last - UNPLUG_DROP)))
		{
			restart(b);
		}
	}

	b->v[b->head] = b->slotMax;
	b->seq[b->head] = b->slotSeq;
	b->head = (uint8_t)((b->head + 1U) % BATT_RT_SLOTS);
	if (b->count < BATT_RT_SLOTS)
	{
		b->count++;
	}

	if (b->count < BATT_RT_MIN_SLOTS)
	{
		return;
	}

	float est = estimateMinutes(b);

	if (est < 0.0f)
	{
		b->shownMin = -1.0f;
	}
	else if (b->shownMin < 0.0f)
	{
		b->shownMin = est;
	}
	else
	{
		b->shownMin += ((est - b->shownMin) * SHOWN_SMOOTHING);
	}

	if (b->shownMin > (float)BATT_RT_MAX_MINUTES)
	{
		b->shownMin = (float)BATT_RT_MAX_MINUTES;
	}
}

void batteryRuntimeAddSample(batteryRuntime_t *b, uint32_t nowMs, int16_t v, bool exclude)
{
	if (b->started == false)
	{
		b->started = true;
		b->slotStartMs = nowMs;
		b->slotMax = -1;
	}

	uint32_t elapsed = (nowMs - b->slotStartMs);

	if (elapsed >= BATT_RT_SLOT_MS)
	{
		uint32_t slots = (elapsed / BATT_RT_SLOT_MS);

		closeSlot(b, slots);
		if (slots > GAP_SLOTS)
		{
			restart(b);
			b->slotStartMs = nowMs;
			b->slotSeq++;
		}
		else
		{
			b->slotStartMs += (slots * BATT_RT_SLOT_MS);
			b->slotSeq = (uint8_t)(b->slotSeq + slots);
		}
		b->slotMax = -1;
	}

	if ((exclude == false) && (v > b->slotMax))
	{
		b->slotMax = v;
	}
}

int batteryRuntimeMinutes(const batteryRuntime_t *b)
{
	if ((b->count < BATT_RT_MIN_SLOTS) || (b->shownMin < 0.0f))
	{
		return BATT_RT_UNKNOWN;
	}
	return (int)(b->shownMin + 0.5f);
}

void batteryRuntimeFormat(int minutes, char *buf, size_t size)
{
	if (minutes < 0)
	{
		snprintf(buf, size, "--");
	}
	else if (minutes >= 595)
	{
		snprintf(buf, size, "~%dh", ((minutes + 30) / 60));
	}
	else if (minutes >= 60)
	{
		int r = (((minutes + 5) / 10) * 10);

		snprintf(buf, size, "~%dh%02d", (r / 60), (r % 60));
	}
	else if (minutes >= 5)
	{
		snprintf(buf, size, "~%dm", (((minutes + 2) / 5) * 5));
	}
	else
	{
		snprintf(buf, size, "<5m");
	}
}
