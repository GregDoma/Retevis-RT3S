/*
 * Fork: EMCOM net control roster (main menu > Net control), by Marcin SP7TQ.
 *
 * Hardware independent part (host tested in tests/aprs_rx/test_net.c): the list of the
 * stations heard since the net was started, their check-in / last heard times (seconds
 * since the start, 18 h range), the silence ("quiet") alarm and the texts of the QSO / NET
 * event log records.
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#ifndef _OPENGD77_NET_ROSTER_H_
#define _OPENGD77_NET_ROSTER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define NET_ROSTER_MAX       20U
#define NET_CALL_LEN         9U          // stored without the NUL when 9 long ("SP5ABC-15")
#define NET_CALL_BUF         (NET_CALL_LEN + 1U)

// Station keys: a DMR ID (24 bits), an APRS callsign hash, or an FM manual check-in number
#define NET_KEY_APRS         0x80000000U
#define NET_KEY_FM           0x40000000U
#define NET_KEY_IS_FM(k)     (((k) & 0xC0000000U) == NET_KEY_FM)

#define NET_FLAG_QUIET       0x01U       // the silence alarm fired (re-armed when heard again)

typedef struct __attribute__((packed))
{
	uint32_t key;
	uint16_t inS;                 // check-in, seconds since the net start
	uint16_t lastS;               // last heard, seconds since the net start
	char     call[NET_CALL_LEN];
	uint8_t  flags;
} netEntry_t;                     // 18 bytes

typedef struct
{
	netEntry_t e[NET_ROSTER_MAX];
	uint8_t    count;
	uint8_t    fmLast;            // last FM#n number given
} netRoster_t;

typedef enum
{
	NET_HEARD_FULL = -1,          // new station, the roster is full
	NET_HEARD_AGAIN = 0,
	NET_HEARD_NEW = 1
} netHeard_t;

void netRosterInit(netRoster_t *r);
int netRosterFind(const netRoster_t *r, uint32_t key);  // -1: not in the roster
// A station was heard at 'atS': adds it (check-in) or updates its last heard time and re-arms
// its alarm. 'call' replaces a numeric (DMR ID) name. 'index' (may be NULL) gets its position.
netHeard_t netRosterHeard(netRoster_t *r, uint32_t key, const char *call, uint16_t atS, int *index);
int netRosterAddFm(netRoster_t *r, uint16_t atS);       // "FM#n" check-in, returns its index, -1: full
// true when the station has not been heard for 'limitS' seconds or more
bool netEntryIsQuiet(const netEntry_t *e, uint16_t nowS, uint16_t limitS);
// Next station whose alarm is due: marks it fired and returns its index, -1: none
int netRosterQuietNext(netRoster_t *r, uint16_t nowS, uint16_t limitS);

void netEntryCall(const netEntry_t *e, char *buf);      // NUL terminated name, NET_CALL_BUF bytes
uint32_t netAprsKey(const char *call);
uint32_t netDmrKey(uint32_t id);
uint16_t netSeconds(uint32_t elapsedMs);                // clamped to 65535
// "<station> <report>[ <code>]" in at most size - 1 chars, the station part truncated (code: NULL = none)
void netQsoText(char *buf, size_t size, const char *station, const char *report, const char *code);
// Age "45s", "12m", "3h", "18h"
void netAgeText(char *buf, size_t size, uint16_t ageS);

#endif
