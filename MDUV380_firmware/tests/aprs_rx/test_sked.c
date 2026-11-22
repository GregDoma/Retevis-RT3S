// Host test for skedMath.c (comms schedule reminders): next start, on / off, wrap at the hour and midnight, editing steps
#include <stdio.h>
#include <string.h>
#include "functions/skedMath.h"

static int fails;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond); fails++; } } while (0)

#define DAY0 1795996800U // 2026-11-30 00:00:00 UTC
#define AT(h, m, s) (DAY0 + ((h) * 3600U) + ((m) * 60U) + (s))

static skedTime_t mk(uint16_t period, uint16_t offset, uint8_t duration)
{
	skedTime_t s;

	skedTimeDefault(&s);
	s.periodMin = period;
	s.offsetMin = offset;
	s.durationMin = duration;
	return s;
}

int main(void)
{
	char buf[32];

	CHECK((DAY0 % 86400U) == 0U);

	// every hour at :00, 10 min
	skedTime_t h = mk(60, 0, 10);
	CHECK(skedTimeIsValid(&h));
	CHECK(skedSecondsToStart(AT(12, 0, 0), &h) == 0U);
	CHECK(skedSecondsToStart(AT(12, 59, 0), &h) == 60U);     // the 1 min warning
	CHECK(skedSecondsToStart(AT(12, 0, 1), &h) == 3599U);
	CHECK(skedSecondsToStart(AT(23, 59, 0), &h) == 60U);     // wraps at midnight
	CHECK(skedSecondsToStart(AT(23, 59, 59), &h) == 1U);
	CHECK(skedIsOn(AT(12, 9, 59), &h));
	CHECK(skedIsOn(AT(12, 10, 0), &h) == false);
	CHECK(skedIsOn(AT(0, 5, 0), &h));
	CHECK(skedSecondsSinceStart(AT(0, 5, 0), &h) == 300U);

	// every 30 min from :15
	skedTime_t t = mk(30, 15, 5);
	CHECK(skedSecondsToStart(AT(10, 0, 0), &t) == (15U * 60U));
	CHECK(skedSecondsToStart(AT(10, 15, 0), &t) == 0U);
	CHECK(skedSecondsToStart(AT(10, 16, 0), &t) == (29U * 60U)); // next at :45
	CHECK(skedSecondsToStart(AT(10, 44, 0), &t) == 60U);
	CHECK(skedSecondsToStart(AT(23, 50, 0), &t) == (25U * 60U)); // 00:15 next day
	CHECK(skedIsOn(AT(10, 49, 0), &t));
	CHECK(skedIsOn(AT(10, 50, 0), &t) == false);
	CHECK(skedIsOn(AT(10, 5, 0), &t) == false);

	// daily at 18:00 UTC
	skedTime_t d = mk(1440, 1080, 15);
	CHECK(skedSecondsToStart(AT(17, 59, 0), &d) == 60U);
	CHECK(skedSecondsToStart(AT(18, 0, 0), &d) == 0U);
	CHECK(skedSecondsToStart(AT(18, 0, 1), &d) == (86400U - 1U));
	CHECK(skedSecondsToStart(AT(0, 0, 0), &d) == (18U * 3600U));
	CHECK(skedIsOn(AT(18, 14, 59), &d));
	CHECK(skedIsOn(AT(18, 15, 0), &d) == false);
	CHECK(skedSecondsToStart(AT(18, 0, 0) + 86400U, &d) == 0U); // the next day too

	// daily at 23:55, 10 min: on across midnight
	skedTime_t m = mk(1440, 1435, 10);
	CHECK(skedIsOn(AT(23, 58, 0), &m));
	CHECK(skedIsOn(AT(0, 4, 59) + 86400U, &m));
	CHECK(skedIsOn(AT(0, 5, 0) + 86400U, &m) == false);
	CHECK(skedSecondsToStart(AT(0, 5, 0), &m) == ((23U * 3600U) + (50U * 60U)));

	// every 2 h from 01:00
	skedTime_t e = mk(120, 60, 10);
	CHECK(skedSecondsToStart(AT(0, 59, 0), &e) == 60U);
	CHECK(skedSecondsToStart(AT(2, 0, 0), &e) == 3600U);
	CHECK(skedSecondsToStart(AT(23, 30, 0), &e) == (90U * 60U)); // 01:00

	// validity
	skedTime_t bad = mk(45, 0, 5);
	CHECK(skedTimeIsValid(&bad) == false); // 45 does not divide a day
	bad = mk(60, 60, 5);
	CHECK(skedTimeIsValid(&bad) == false);
	bad = mk(15, 0, 20);
	CHECK(skedTimeIsValid(&bad) == false);
	bad = mk(60, 0, 0);
	CHECK(skedTimeIsValid(&bad) == false);
	bad = mk(60, 0, 5);
	bad.tune = SKED_TUNE_MAX;
	CHECK(skedTimeIsValid(&bad) == false);

	// steps
	CHECK(skedPeriodStep(60, 1) == 120U);
	CHECK(skedPeriodStep(1440, 1) == 15U);
	CHECK(skedPeriodStep(15, -1) == 1440U);
	CHECK(skedOffsetStep(60, 0, 1) == 5U);
	CHECK(skedOffsetStep(60, 55, 1) == 0U);
	CHECK(skedOffsetStep(60, 0, -1) == 55U);
	CHECK(skedOffsetStep(60, 7, 1) == 10U);
	CHECK(skedOffsetStep(60, 7, -1) == 5U);
	CHECK(skedOffsetStep(1440, 0, -1) == 1425U);
	CHECK(skedOffsetStep(1440, 1065, 1) == 1080U);
	CHECK(skedOffsetStep(30, 25, 1) == 0U);
	CHECK(skedDurationStep(60, 10, 1) == 15U);
	CHECK(skedDurationStep(60, 60, 1) == 1U);
	CHECK(skedDurationStep(15, 15, 1) == 1U);   // not longer than the period
	CHECK(skedDurationStep(15, 1, -1) == 15U);
	CHECK(skedDurationStep(1440, 1, -1) == 60U);
	CHECK(skedDurationStep(60, 7, -1) == 5U);
	CHECK(skedDurationStep(60, 7, 1) == 10U);

	skedTime_t f = mk(60, 45, 30);
	f.periodMin = skedPeriodStep(60, -1); // 30
	skedTimeFix(&f);
	CHECK(f.periodMin == 30U && f.offsetMin == 15U && f.durationMin == 30U && skedTimeIsValid(&f));
	f.periodMin = 15U;
	skedTimeFix(&f);
	CHECK(f.offsetMin == 0U && f.durationMin == 15U && skedTimeIsValid(&f));

	// text
	skedFormatWhen(&h, buf, sizeof(buf));
	CHECK(strcmp(buf, "HOURLY :00") == 0);
	skedFormatWhen(&t, buf, sizeof(buf));
	CHECK(strcmp(buf, "EVERY 30M :15") == 0);
	skedFormatWhen(&d, buf, sizeof(buf));
	CHECK(strcmp(buf, "DAILY 18:00") == 0);
	skedFormatWhen(&e, buf, sizeof(buf));
	CHECK(strcmp(buf, "EVERY 2H 01:00") == 0);
	skedFormatCountdown(65, buf, sizeof(buf));
	CHECK(strcmp(buf, "01:05") == 0);
	skedFormatCountdown(5999, buf, sizeof(buf));
	CHECK(strcmp(buf, "99:59") == 0);
	skedFormatCountdown((5U * 3600U) + (12U * 60U) + 30U, buf, sizeof(buf));
	CHECK(strcmp(buf, "5H12") == 0);

	printf("test_sked: %s\n", (fails ? "FAILED" : "all passed"));
	return (fails ? 1 : 0);
}
