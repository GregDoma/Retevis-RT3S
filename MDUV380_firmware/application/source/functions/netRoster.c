/*
 * Fork: EMCOM net control roster, by Marcin SP7TQ. See netRoster.h.
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#include <string.h>
#include <stdio.h>
#include "functions/netRoster.h"

void netRosterInit(netRoster_t *r)
{
	memset(r, 0, sizeof(netRoster_t));
}

int netRosterFind(const netRoster_t *r, uint32_t key)
{
	for (int i = 0; i < (int)r->count; i++)
	{
		if (r->e[i].key == key)
		{
			return i;
		}
	}
	return -1;
}

static bool isNumeric(const char *s, size_t max)
{
	size_t n = 0U;

	for (; (n < max) && s[n]; n++)
	{
		if ((s[n] < '0') || (s[n] > '9'))
		{
			return false;
		}
	}
	return (n > 0U);
}

static void setCall(netEntry_t *e, const char *call)
{
	memset(e->call, 0, NET_CALL_LEN);
	strncpy(e->call, call, NET_CALL_LEN);
}

netHeard_t netRosterHeard(netRoster_t *r, uint32_t key, const char *call, uint16_t atS, int *index)
{
	int i = netRosterFind(r, key);

	if (i >= 0)
	{
		netEntry_t *e = &r->e[i];

		if (atS > e->lastS)
		{
			e->lastS = atS;
		}
		e->flags &= (uint8_t)~NET_FLAG_QUIET;
		// a DMR ID shown until the callsign / talker alias is known
		if ((call != NULL) && call[0] && isNumeric(e->call, NET_CALL_LEN) && (isNumeric(call, NET_CALL_LEN) == false))
		{
			setCall(e, call);
		}
		if (index)
		{
			*index = i;
		}
		return NET_HEARD_AGAIN;
	}

	if (r->count >= NET_ROSTER_MAX)
	{
		if (index)
		{
			*index = -1;
		}
		return NET_HEARD_FULL;
	}

	i = r->count++;
	r->e[i].key = key;
	r->e[i].inS = atS;
	r->e[i].lastS = atS;
	r->e[i].flags = 0U;
	setCall(&r->e[i], ((call != NULL) ? call : ""));
	if (index)
	{
		*index = i;
	}
	return NET_HEARD_NEW;
}

int netRosterAddFm(netRoster_t *r, uint16_t atS)
{
	char call[NET_CALL_BUF];
	int index;

	if (r->count >= NET_ROSTER_MAX)
	{
		return -1;
	}
	r->fmLast++;
	snprintf(call, sizeof(call), "FM#%u", (unsigned)r->fmLast);
	netRosterHeard(r, (NET_KEY_FM | r->fmLast), call, atS, &index);
	return index;
}

bool netEntryIsQuiet(const netEntry_t *e, uint16_t nowS, uint16_t limitS)
{
	return ((nowS >= e->lastS) && ((uint16_t)(nowS - e->lastS) >= limitS));
}

int netRosterQuietNext(netRoster_t *r, uint16_t nowS, uint16_t limitS)
{
	for (int i = 0; i < (int)r->count; i++)
	{
		netEntry_t *e = &r->e[i];

		if (((e->flags & NET_FLAG_QUIET) == 0U) && netEntryIsQuiet(e, nowS, limitS))
		{
			e->flags |= NET_FLAG_QUIET;
			return i;
		}
	}
	return -1;
}

void netEntryCall(const netEntry_t *e, char *buf)
{
	memcpy(buf, e->call, NET_CALL_LEN);
	buf[NET_CALL_LEN] = 0;
}

uint32_t netAprsKey(const char *call)
{
	uint32_t h = 2166136261U; // FNV-1a

	while (*call)
	{
		h = ((h ^ (uint8_t)*call++) * 16777619U);
	}
	return (NET_KEY_APRS | (h & 0x7FFFFFFFU));
}

uint32_t netDmrKey(uint32_t id)
{
	return (id & 0xFFFFFFU);
}

uint16_t netSeconds(uint32_t elapsedMs)
{
	uint32_t s = (elapsedMs / 1000U);

	return (uint16_t)((s > 65535U) ? 65535U : s);
}

void netQsoText(char *buf, size_t size, const char *station, const char *report, const char *code)
{
	size_t tail = (1U + strlen(report) + ((code != NULL) ? (1U + strlen(code)) : 0U));
	int max = (int)size - 1 - (int)tail;

	if (size == 0U)
	{
		return;
	}
	if (max < 1)
	{
		max = 1;
	}
	if (code != NULL)
	{
		snprintf(buf, size, "%.*s %s %s", max, station, report, code);
	}
	else
	{
		snprintf(buf, size, "%.*s %s", max, station, report);
	}
}

void netAgeText(char *buf, size_t size, uint16_t ageS)
{
	if (ageS < 60U)
	{
		snprintf(buf, size, "%us", (unsigned)ageS);
	}
	else if (ageS < 3600U)
	{
		snprintf(buf, size, "%um", (unsigned)(ageS / 60U));
	}
	else
	{
		snprintf(buf, size, "%uh", (unsigned)(ageS / 3600U));
	}
}
