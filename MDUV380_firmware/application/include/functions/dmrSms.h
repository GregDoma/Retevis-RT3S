/*
 * Fork: DMR SMS receive and inbox, by Marcin SP7TQ. RX path ported from
 * dondch/OpenGD77-AES256 (cleartext only); the PDU / text logic is functions/dmrSmsPdu.c.
 *
 * Interrupt side: the HR-C6000 RX interrupt hands every CRC valid data burst (header, rate 1/2,
 * rate 3/4) to dmrSmsIsrBurst(); a completed PDU waits for dmrSmsTick() (main loop), which
 * decodes it, files it in the inbox (SPI flash), logs it and notifies. Received cleartext only;
 * an encrypted PDU is dropped. Sending is not implemented.
 *
 * Inbox: 32 slots of one flash page (256 bytes) in two 4 KB sectors at DMR_SMS_FLASH_ADDRESS,
 * a ring: entering a sector erases it, so the newest 16..32 messages are kept. Read / deleted
 * marks clear bits of the record in place (no erase).
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#ifndef _OPENGD77_DMR_SMS_H_
#define _OPENGD77_DMR_SMS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "functions/dmrSmsPdu.h"

#if defined(HAS_DMR_SMS)
#include "functions/sstvStore.h"

// SPI flash (16 MB) use from 14 MB up: event log 64 KB (functions/eventLog.h), GPS NMEA log
// (interfaces/gps.c), this inbox 8 KB, SSTV pictures 320 KB (functions/sstvStore.h), field
// tools data 64 KB (functions/fieldData.h). The inbox was taken from the end of the GPS log.
#define DMR_SMS_FLASH_SIZE      (8U * 1024U)
#define DMR_SMS_FLASH_ADDRESS   (SSTV_STORE_FLASH_ADDRESS - DMR_SMS_FLASH_SIZE) // 0xF9E000, the GPS log ends here
#define DMR_SMS_SLOTS           32U

#define DMR_SMS_FLAG_LIVE       0x01U  // cleared: deleted
#define DMR_SMS_FLAG_UNREAD     0x02U  // cleared: read
#define DMR_SMS_FLAG_GROUP      0x04U  // dst is a talkgroup

typedef struct
{
	uint32_t seq;
	uint32_t time;    // UTC seconds, 0: no clock
	uint32_t src;
	uint32_t dst;
	uint8_t  flags;
	uint8_t  len;
	char     text[DMR_SMS_TEXT_MAX + 1];
} dmrSmsMessage_t;

// main loop
void dmrSmsTick(void);
uint32_t dmrSmsCount(void);
uint32_t dmrSmsUnreadCount(void);
uint32_t dmrSmsSerial(void);                           // changes when a message arrives or goes
bool dmrSmsGet(uint32_t indexFromNewest, dmrSmsMessage_t *msg);
void dmrSmsMarkRead(uint32_t indexFromNewest);
void dmrSmsDelete(uint32_t indexFromNewest);
void dmrSmsDeleteAll(void);
void dmrSmsPeerName(uint32_t id, char *buf, size_t size); // contact / DMR ID database callsign, else "ID n"

// HR-C6000 interrupt
void dmrSmsIsrBurst(int dataType, const uint8_t *payload);
void dmrSmsIsrCallEnd(void);                           // CRC valid Terminator
bool dmrSmsIsrAckTake(uint8_t out[12]);                // the Response header owed for the PDU just completed

#endif
#endif
