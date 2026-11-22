/*
 * Fork: EMCOM net control (main menu > Net control), by Marcin SP7TQ.
 *
 * While a net runs, netTick() (main loop, once per second) checks in every station
 * heard since the start: DMR from the last heard list (callsign, else the DMR ID),
 * APRS from the station list. Each new station is logged as "IN <station>"; a station
 * not heard for the quiet limit fires its silence alarm once ("QUIET <station>" in the
 * log, a warning card and a beep), re-armed when it is heard again. FM speakers cannot
 * be identified: the operator adds "FM#n" check-ins by hand.
 * The roster lives in RAM (netRoster.h, 18 stations), the events in the event log.
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#ifndef _OPENGD77_NET_CONTROL_H_
#define _OPENGD77_NET_CONTROL_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "functions/eventLog.h"
#include "functions/fieldData.h"
#include "functions/netRoster.h"

#if defined(HAS_EVENT_LOG) && defined(HAS_FIELD_TOOLS)
#define HAS_NET_CONTROL

#include "user_interface/uiGlobals.h"

bool netIsRunning(void);
void netStart(void);                       // logs "START[ <channel>]"
void netEnd(void);                         // logs "END <n>"
const netRoster_t *netGetRoster(void);     // the last net's stations stay after the end
uint16_t netNowS(void);                    // seconds since the start
uint32_t netStartUtc(void);                // UTC seconds, 0: no clock at the start
uint16_t netQuietLimitS(void);
void netQuietLimitCycle(void);             // 5 / 10 / 15 / 30 min
uint32_t netSerial(void);                  // changes when the roster changes
int netAddFm(void);                        // "FM#n" check-in now, -1: roster full
void netMarkHeard(int index);              // the operator heard this station now
void netLogQso(const char *station);       // QSO "<station> 59" + "QSO logged" card
void netStationName(const LinkItem_t *item, char *buf, size_t size); // DMR: callsign, else the ID
void netChannelName(char *buf, size_t size); // channel name, VFO: frequency
void netTick(void);                        // main loop

#endif
#endif
