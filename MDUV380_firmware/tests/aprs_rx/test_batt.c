// Host test for batteryRuntime.c (battery "time left" estimate) with synthetic discharge curves
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "functions/batteryRuntime.h"

static int fails;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond); fails++; } } while (0)

#define V_EMPTY 640  // 6.40 V (CUTOFF_VOLTAGE_UPPER_HYST)

static unsigned rnd = 12345U;
static double noise(double amp) // uniform -amp..amp
{
	rnd = (rnd * 1103515245U) + 12345U;
	return (((double)((rnd >> 8) & 0xFFFF) / 65535.0) * 2.0 - 1.0) * amp;
}

static int16_t volts(double v)
{
	return (int16_t)lround(v * 100.0);
}

typedef struct
{
	int unknownAfter20;   // unknown results after 20 min of data
	int maxStep;          // largest change between two minutes
	double worstErr;      // largest relative error after 45 min
	int last;
} result_t;

// v0 at t = 0, drain in V/h, run for `hours`; TX: a burst of txS seconds every txEveryS seconds
// sagging by txSag V (excluded from the samples until 5 s after the end, recovering over ~20 s)
static result_t run(double v0, double drainVh, double hours, int txEveryS, int txS, double txSag, double noiseV, uint32_t startMs)
{
	batteryRuntime_t b;
	result_t r = { 0, 0, 0.0, BATT_RT_UNKNOWN };
	int prevMin = BATT_RT_UNKNOWN;

	batteryRuntimeInit(&b, V_EMPTY);
	for (int t = 0; t < (int)(hours * 3600.0); t++)
	{
		double v = v0 - (drainVh * t / 3600.0);
		bool exclude = false;

		if (v < 6.2)
		{
			break;
		}
		if (txEveryS > 0)
		{
			int ph = (t % txEveryS);

			if (ph < txS)
			{
				v -= txSag;
			}
			else if (ph < (txS + 40))
			{
				v -= txSag * exp(-(ph - txS) / 6.0);
			}
			exclude = (ph < (txS + 5));
		}
		v += noise(noiseV);

		batteryRuntimeAddSample(&b, (startMs + (uint32_t)t * 1000U), volts(v), exclude);

		if ((t % 60) == 0)
		{
			int m = batteryRuntimeMinutes(&b);

			if ((t >= (20 * 60)) && (m == BATT_RT_UNKNOWN))
			{
				r.unknownAfter20++;
			}
			if ((m != BATT_RT_UNKNOWN) && (prevMin != BATT_RT_UNKNOWN) && (abs(m - prevMin) > r.maxStep))
			{
				r.maxStep = abs(m - prevMin);
			}
			if ((t >= (45 * 60)) && (m != BATT_RT_UNKNOWN) && (drainVh > 0.0))
			{
				double truth = ((v0 - (drainVh * t / 3600.0)) - 6.40) / drainVh * 60.0;

				if (truth > 30.0)
				{
					double err = fabs(m - truth) / truth;

					if (err > r.worstErr)
					{
						r.worstErr = err;
					}
				}
			}
			prevMin = m;
			r.last = m;
		}
	}
	return r;
}

int main(void)
{
	char buf[12];
	batteryRuntime_t b;
	result_t r;

	CHECK(sizeof(batteryRuntime_t) == 56);

	// --- format
	batteryRuntimeFormat(BATT_RT_UNKNOWN, buf, sizeof(buf)); CHECK(strcmp(buf, "--") == 0);
	batteryRuntimeFormat(200, buf, sizeof(buf)); CHECK(strcmp(buf, "~3h20") == 0);
	batteryRuntimeFormat(204, buf, sizeof(buf)); CHECK(strcmp(buf, "~3h20") == 0);
	batteryRuntimeFormat(206, buf, sizeof(buf)); CHECK(strcmp(buf, "~3h30") == 0);
	batteryRuntimeFormat(60, buf, sizeof(buf)); CHECK(strcmp(buf, "~1h00") == 0);
	batteryRuntimeFormat(44, buf, sizeof(buf)); CHECK(strcmp(buf, "~45m") == 0);
	batteryRuntimeFormat(3, buf, sizeof(buf)); CHECK(strcmp(buf, "<5m") == 0);
	batteryRuntimeFormat(594, buf, sizeof(buf)); CHECK(strcmp(buf, "~9h50") == 0);
	batteryRuntimeFormat(600, buf, sizeof(buf)); CHECK(strcmp(buf, "~10h") == 0);
	batteryRuntimeFormat(BATT_RT_MAX_MINUTES, buf, sizeof(buf)); CHECK(strcmp(buf, "~99h") == 0);

	// --- too little data: unknown for the first 15 minutes
	batteryRuntimeInit(&b, V_EMPTY);
	for (int t = 0; t < (14 * 60); t++)
	{
		batteryRuntimeAddSample(&b, (uint32_t)t * 1000U, volts(8.0 - 0.2 * t / 3600.0), false);
	}
	CHECK(batteryRuntimeMinutes(&b) == BATT_RT_UNKNOWN);

	// --- constant drain, 0.18 V/h from 8.0 V (~8.9 h), small noise
	r = run(8.0, 0.18, 6.0, 0, 0, 0.0, 0.02, 0U);
	printf("constant drain: worst error %.1f%%, max step %d min, unknown %d\n", r.worstErr * 100.0, r.maxStep, r.unknownAfter20);
	CHECK(r.worstErr < 0.15);
	CHECK(r.maxStep <= 15);
	CHECK(r.unknownAfter20 == 0);

	// --- fast drain (1.0 V/h, ~1.6 h): minutes, not hours
	r = run(8.0, 1.0, 1.5, 0, 0, 0.0, 0.02, 0U);
	printf("fast drain: worst error %.1f%%, max step %d min, last %d\n", r.worstErr * 100.0, r.maxStep, r.last);
	CHECK(r.worstErr < 0.20);
	CHECK(r.last > 0 && r.last < 30);

	// --- TX bursts: 40 s every 4 minutes, 0.35 V sag, drain 0.25 V/h, the clock near its wrap
	r = run(8.1, 0.25, 5.0, 240, 40, 0.35, 0.02, 0xFFF00000U);
	printf("TX bursts: worst error %.1f%%, max step %d min, unknown %d\n", r.worstErr * 100.0, r.maxStep, r.unknownAfter20);
	CHECK(r.worstErr < 0.20);
	CHECK(r.maxStep <= 15);
	CHECK(r.unknownAfter20 == 0);

	// --- long TX: 2.5 minutes in every 4 (whole slots without samples)
	r = run(8.1, 0.30, 4.0, 240, 150, 0.40, 0.02, 0U);
	printf("long TX: worst error %.1f%%, max step %d min\n", r.worstErr * 100.0, r.maxStep);
	CHECK(r.worstErr < 0.25);

	// --- heavy noise (+-0.06 V)
	r = run(7.9, 0.20, 5.0, 0, 0, 0.0, 0.06, 0U);
	printf("noise: worst error %.1f%%, max step %d min\n", r.worstErr * 100.0, r.maxStep);
	CHECK(r.worstErr < 0.20);
	CHECK(r.maxStep <= 15);

	// --- flat battery (mains supply / tiny drain): clamped, never a short time
	r = run(7.8, 0.0, 3.0, 0, 0, 0.0, 0.03, 0U);
	printf("flat: last %d min\n", r.last);
	CHECK(r.last >= (20 * 60));

	// --- charging: rising 0.4 V/h -> unknown
	r = run(6.9, -0.4, 2.0, 0, 0, 0.0, 0.02, 0U);
	printf("charging: last %d, unknown %d\n", r.last, r.unknownAfter20);
	CHECK(r.last == BATT_RT_UNKNOWN);

	// --- discharging, then the charger is plugged in (+0.3 V step): unknown at once
	batteryRuntimeInit(&b, V_EMPTY);
	for (int t = 0; t < (40 * 60); t++)
	{
		batteryRuntimeAddSample(&b, (uint32_t)t * 1000U, volts(7.6 - 0.2 * t / 3600.0), false);
	}
	CHECK(batteryRuntimeMinutes(&b) != BATT_RT_UNKNOWN);
	for (int t = (40 * 60); t < (46 * 60); t++)
	{
		batteryRuntimeAddSample(&b, (uint32_t)t * 1000U, volts(7.9 + 0.4 * (t - 2400) / 3600.0), false);
	}
	CHECK(batteryRuntimeMinutes(&b) == BATT_RT_UNKNOWN);

	// --- a long gap (suspend): start again
	batteryRuntimeInit(&b, V_EMPTY);
	for (int t = 0; t < (40 * 60); t++)
	{
		batteryRuntimeAddSample(&b, (uint32_t)t * 1000U, volts(7.6 - 0.2 * t / 3600.0), false);
	}
	CHECK(batteryRuntimeMinutes(&b) != BATT_RT_UNKNOWN);
	batteryRuntimeAddSample(&b, (uint32_t)(5 * 3600) * 1000U, volts(7.4), false);
	CHECK(batteryRuntimeMinutes(&b) == BATT_RT_UNKNOWN);

	// --- empty: 0 minutes, not unknown
	r = run(6.6, 0.5, 1.0, 0, 0, 0.0, 0.01, 0U);
	printf("near empty: last %d\n", r.last);
	CHECK(r.last >= 0 && r.last <= 5);

	if (fails)
	{
		printf("test_batt: %d FAILED\n", fails);
		return 1;
	}
	printf("test_batt: all passed\n");
	return 0;
}
