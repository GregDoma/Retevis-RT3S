/*
 * Fork: comms schedule timing (see functions/skedMath.h).
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#include <stdio.h>
#include "functions/skedMath.h"

static const uint16_t periods[] = { 15U, 20U, 30U, 60U, 120U, 180U, 240U, 360U, 480U, 720U, 1440U };
static const uint8_t durations[] = { 1U, 2U, 3U, 5U, 10U, 15U, 20U, 30U, 45U, 60U };
#define N_PERIODS   ((int)(sizeof(periods) / sizeof(periods[0])))
#define N_DURATIONS ((int)(sizeof(durations) / sizeof(durations[0])))

static int periodIndex(uint16_t p)
{
	for (int i = 0; i < N_PERIODS; i++)
	{
		if (periods[i] == p)
		{
			return i;
		}
	}
	return -1;
}

bool skedTimeIsValid(const skedTime_t *s)
{
	return ((periodIndex(s->periodMin) >= 0) && (s->offsetMin < s->periodMin) && (s->durationMin >= 1U) &&
			(s->durationMin <= s->periodMin) && (s->tune < SKED_TUNE_MAX) && (s->enabled <= 1U));
}

void skedTimeDefault(skedTime_t *s)
{
	s->periodMin = 60U;
	s->offsetMin = 0U;
	s->durationMin = 10U;
	s->tune = SKED_TUNE_NONE;
	s->enabled = 1U;
	s->reserved = 0U;
}

uint32_t skedSecondsSinceStart(uint32_t utc, const skedTime_t *s)
{
	uint32_t periodS = ((uint32_t)s->periodMin * 60U);
	uint32_t dayS = (utc % (SKED_DAY_MIN * 60U));
	uint32_t offS = ((uint32_t)s->offsetMin * 60U);

	// the period divides the day: the position in the day is enough
	return (((dayS + (SKED_DAY_MIN * 60U)) - offS) % periodS);
}

uint32_t skedSecondsToStart(uint32_t utc, const skedTime_t *s)
{
	uint32_t since = skedSecondsSinceStart(utc, s);

	return ((since == 0U) ? 0U : (((uint32_t)s->periodMin * 60U) - since));
}

bool skedIsOn(uint32_t utc, const skedTime_t *s)
{
	return (skedSecondsSinceStart(utc, s) < ((uint32_t)s->durationMin * 60U));
}

uint16_t skedPeriodStep(uint16_t periodMin, int dir)
{
	int i = periodIndex(periodMin);

	if (i < 0)
	{
		return 60U;
	}
	return periods[(i + N_PERIODS + ((dir > 0) ? 1 : -1)) % N_PERIODS];
}

uint16_t skedOffsetStep(uint16_t periodMin, uint16_t offsetMin, int dir)
{
	uint16_t step = ((periodMin <= 60U) ? 5U : 15U);

	if (dir > 0)
	{
		offsetMin = (uint16_t)(((offsetMin / step) * step) + step); // back on the step grid
		return ((offsetMin >= periodMin) ? 0U : offsetMin);
	}

	if (offsetMin == 0U)
	{
		return (uint16_t)(((periodMin - 1U) / step) * step);
	}
	return (uint16_t)(((offsetMin - 1U) / step) * step);
}

uint8_t skedDurationStep(uint16_t periodMin, uint8_t durationMin, int dir)
{
	if (dir > 0)
	{
		for (int i = 0; i < N_DURATIONS; i++)
		{
			if ((durations[i] > durationMin) && (durations[i] <= periodMin))
			{
				return durations[i];
			}
		}
		return durations[0]; // wrap to the shortest
	}

	for (int i = (N_DURATIONS - 1); i >= 0; i--)
	{
		if ((durations[i] < durationMin) && (durations[i] <= periodMin))
		{
			return durations[i];
		}
	}

	for (int i = (N_DURATIONS - 1); i >= 0; i--) // wrap to the longest that fits the period
	{
		if (durations[i] <= periodMin)
		{
			return durations[i];
		}
	}
	return durations[0];
}

void skedTimeFix(skedTime_t *s)
{
	if (periodIndex(s->periodMin) < 0)
	{
		s->periodMin = 60U;
	}
	s->offsetMin = (uint16_t)(s->offsetMin % s->periodMin);
	if (s->durationMin > s->periodMin)
	{
		s->durationMin = (uint8_t)((s->periodMin > 60U) ? 60U : s->periodMin);
	}
	if (s->durationMin == 0U)
	{
		s->durationMin = 1U;
	}
}

void skedFormatWhen(const skedTime_t *s, char *buf, size_t size)
{
	unsigned h = (unsigned)(s->offsetMin / 60U), m = (unsigned)(s->offsetMin % 60U);

	if (s->periodMin >= SKED_DAY_MIN)
	{
		snprintf(buf, size, "DAILY %02u:%02u", h, m);
	}
	else if (s->periodMin == 60U)
	{
		snprintf(buf, size, "HOURLY :%02u", m);
	}
	else if (s->periodMin < 60U)
	{
		snprintf(buf, size, "EVERY %uM :%02u", (unsigned)s->periodMin, m);
	}
	else
	{
		snprintf(buf, size, "EVERY %uH %02u:%02u", (unsigned)(s->periodMin / 60U), h, m);
	}
}

void skedFormatCountdown(uint32_t secs, char *buf, size_t size)
{
	if (secs < 6000U)
	{
		snprintf(buf, size, "%02u:%02u", (unsigned)(secs / 60U), (unsigned)(secs % 60U));
	}
	else
	{
		snprintf(buf, size, "%uH%02u", (unsigned)(secs / 3600U), (unsigned)((secs / 60U) % 60U));
	}
}
