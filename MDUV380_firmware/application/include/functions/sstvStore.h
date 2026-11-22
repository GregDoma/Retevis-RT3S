/*
 * Fork: received SSTV pictures kept in the SPI flash (SP7TEAM firmware, Marcin SP7TQ).
 *
 * 8 slots of 40 KB right under the field tools data (functions/fieldData.h), taken
 * from the end of the GPS NMEA log. A slot:
 *   0x000 sstvStoreHeader_t (32 bytes, rest of the page 0xFF), written last, so a save
 *         cut short (power off) leaves no valid slot
 *   0x100 the picture: SSTV_RX_ROWS lines of SSTV_RX_WIDTH pixels, copied from the screen
 *         buffer as is (RGB565, byte order of the display buffer read by CPS area 6)
 * The slots form a ring: a new picture replaces the oldest one.
 *
 * Saving is spread over several calls of sstvStoreTick() (one 4 KB sector each), so the
 * main loop is never blocked for long.
 *
 * Read back over the CPS link (area 1) by the web tools (sp7tq.pl/rt3s/narzedzia, SSTV tab).
 *
 * Use of this source code or binary releases for commercial purposes is
 * strictly forbidden (see license.txt).
 */
#ifndef _OPENGD77_SSTV_STORE_H_
#define _OPENGD77_SSTV_STORE_H_

#include <stdint.h>
#include <stdbool.h>
#include "functions/fieldData.h"

#if defined(HAS_FIELD_TOOLS)
#define HAS_SSTV_STORE

#define SSTV_STORE_SLOTS          8U
#define SSTV_STORE_SLOT_SIZE      (40U * 1024U)
#define SSTV_STORE_SIZE           (SSTV_STORE_SLOTS * SSTV_STORE_SLOT_SIZE)                // 320 KB
#define SSTV_STORE_FLASH_ADDRESS  (FIELD_DATA_FLASH_ADDRESS - SSTV_STORE_SIZE)             // the GPS NMEA log ends here
#define SSTV_STORE_PIXELS_OFFSET  0x100U
#define SSTV_STORE_MAGIC          0x56545353UL // "SSTV"
#define SSTV_STORE_MIN_ROWS       30U          // shorter pictures (signal lost early) are not kept

typedef struct __attribute__((__packed__))
{
	uint32_t magic;
	uint32_t seq;       // 1, 2, 3 ... the highest one is the newest picture
	uint32_t utc;       // UTC seconds, 0 = no clock
	uint32_t rxFreq;    // RX frequency in 10 Hz units (as in the codeplug), 0 = unknown
	uint16_t rows;      // lines received (the rest of the picture is what the screen held)
	uint8_t  width;     // SSTV_RX_WIDTH
	uint8_t  height;    // SSTV_RX_ROWS
	char     mode[12];  // "Robot 36", "Martin 1" ... NUL padded
} sstvStoreHeader_t;    // 32 bytes

// Starts saving the picture held by 'pixels' (SSTV_RX_ROWS * SSTV_RX_WIDTH RGB565, must stay
// unchanged until the save is done). False when a save is still running.
bool sstvStoreBegin(const uint16_t *pixels, uint16_t rows, const char *mode, uint32_t rxFreq, uint32_t utc);
// Does one step of a running save. Returns true on the call that completed it.
bool sstvStoreTick(void);
bool sstvStoreBusy(void);
void sstvStoreFinish(void); // completes a running save at once
uint32_t sstvStoreCount(void); // pictures kept

#endif
#endif
