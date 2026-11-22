// Host test for netRoster.c (net control roster, quiet alarm, QSO / NET log texts)
#include <stdio.h>
#include <string.h>
#include "functions/netRoster.h"

static int fails;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond); fails++; } } while (0)

int main(void)
{
	netRoster_t r;
	char buf[32];
	int idx;

	CHECK(sizeof(netEntry_t) == 18);
	netRosterInit(&r);
	CHECK(r.count == 0);

	// check-in, heard again, the DMR ID replaced by the callsign once known
	CHECK(netRosterHeard(&r, netDmrKey(2601234), "2601234", 10, &idx) == NET_HEARD_NEW);
	CHECK(idx == 0);
	CHECK(netRosterHeard(&r, netDmrKey(0xAB000000U | 2601234), "SP7ABC", 40, &idx) == NET_HEARD_AGAIN);
	CHECK(idx == 0);
	netEntryCall(&r.e[0], buf);
	CHECK(strcmp(buf, "SP7ABC") == 0);
	CHECK(r.e[0].inS == 10 && r.e[0].lastS == 40);
	CHECK(netRosterHeard(&r, netDmrKey(2601234), "2601234", 30, NULL) == NET_HEARD_AGAIN); // older: kept
	netEntryCall(&r.e[0], buf);
	CHECK(strcmp(buf, "SP7ABC") == 0 && r.e[0].lastS == 40);

	// APRS callsigns, a 9 char one kept whole
	CHECK(netAprsKey("SP7ABC-9") != netAprsKey("SP7ABC-1"));
	CHECK((netAprsKey("SP7ABC-9") & 0xC0000000U) == NET_KEY_APRS);
	CHECK(netRosterHeard(&r, netAprsKey("SP5ABC-15"), "SP5ABC-15", 50, &idx) == NET_HEARD_NEW && idx == 1);
	netEntryCall(&r.e[1], buf);
	CHECK(strcmp(buf, "SP5ABC-15") == 0);
	CHECK(netRosterFind(&r, netAprsKey("SP5ABC-15")) == 1);
	CHECK(netRosterFind(&r, netAprsKey("SP5ABC-1")) == -1);

	// FM manual check-ins
	CHECK(netRosterAddFm(&r, 60) == 2);
	CHECK(netRosterAddFm(&r, 61) == 3);
	netEntryCall(&r.e[3], buf);
	CHECK(strcmp(buf, "FM#2") == 0);
	CHECK(NET_KEY_IS_FM(r.e[3].key) && !NET_KEY_IS_FM(r.e[1].key) && !NET_KEY_IS_FM(r.e[0].key));

	// quiet alarm: once per station, re-armed when heard again
	CHECK(!netEntryIsQuiet(&r.e[0], 339, 300));
	CHECK(netEntryIsQuiet(&r.e[0], 340, 300));
	CHECK(netRosterQuietNext(&r, 340, 300) == 0);
	CHECK(netRosterQuietNext(&r, 340, 300) == -1);
	CHECK(netRosterQuietNext(&r, 350, 300) == 1);
	CHECK(netRosterQuietNext(&r, 1000, 300) == 2);
	CHECK(netRosterQuietNext(&r, 1000, 300) == 3);
	CHECK(netRosterQuietNext(&r, 1000, 300) == -1);
	netRosterHeard(&r, netDmrKey(2601234), "SP7ABC", 1001, NULL);
	CHECK((r.e[0].flags & NET_FLAG_QUIET) == 0);
	CHECK(netRosterQuietNext(&r, 1100, 300) == -1);
	CHECK(netRosterQuietNext(&r, 1301, 300) == 0);
	CHECK(!netEntryIsQuiet(&r.e[0], 5, 300)); // clock before the entry: not quiet

	// full roster
	netRosterInit(&r);
	for (unsigned i = 0; i < NET_ROSTER_MAX; i++)
	{
		CHECK(netRosterHeard(&r, netDmrKey(100 + i), "X", 1, NULL) == NET_HEARD_NEW);
	}
	CHECK(netRosterHeard(&r, netDmrKey(99), "Y", 1, &idx) == NET_HEARD_FULL && idx == -1);
	CHECK(netRosterAddFm(&r, 2) == -1);
	CHECK(netRosterHeard(&r, netDmrKey(100), "X", 5, NULL) == NET_HEARD_AGAIN);

	// QSO texts (20 byte record field: 19 chars)
	netQsoText(buf, 20, "SP7ABC-9", "59", NULL);
	CHECK(strcmp(buf, "SP7ABC-9 59") == 0);
	netQsoText(buf, 20, "2601234", "59", "OK");
	CHECK(strcmp(buf, "2601234 59 OK") == 0);
	netQsoText(buf, 20, "FM#3", "5/9", "RPT");
	CHECK(strcmp(buf, "FM#3 5/9 RPT") == 0);
	netQsoText(buf, 20, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "59", "NIL");
	CHECK(strcmp(buf, "ABCDEFGHIJKL 59 NIL") == 0);
	netQsoText(buf, 20, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "59", NULL);
	CHECK(strcmp(buf, "ABCDEFGHIJKLMNOP 59") == 0);

	// ages
	netAgeText(buf, sizeof(buf), 45);
	CHECK(strcmp(buf, "45s") == 0);
	netAgeText(buf, sizeof(buf), 180);
	CHECK(strcmp(buf, "3m") == 0);
	netAgeText(buf, sizeof(buf), 7300);
	CHECK(strcmp(buf, "2h") == 0);
	CHECK(netSeconds(1999) == 1 && netSeconds(100000000U) == 65535);

	printf("%s: %d failure(s)\n", __FILE__, fails);
	return (fails ? 1 : 0);
}
