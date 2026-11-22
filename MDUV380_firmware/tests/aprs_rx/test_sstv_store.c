// Host test for functions/sstvStore.c on a simulated SPI flash (erase sets 0xFF, programming only clears bits).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "functions/sstvStore.h"
#include "functions/sstvRx.h"

#define FLASH_SIZE (16U * 1024U * 1024U)
static uint8_t *flash;
static int fails = 0;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

bool SPI_Flash_read(uint32_t a, uint8_t *b, int n) { memcpy(b, &flash[a], n); return true; }
static int failErase = 0;
bool SPI_Flash_eraseSector(uint32_t a) { if (failErase && --failErase == 0) return false; CHECK((a % 4096U) == 0U); CHECK(a >= SSTV_STORE_FLASH_ADDRESS && a < FIELD_DATA_FLASH_ADDRESS); memset(&flash[a & ~4095U], 0xFF, 4096); return true; }
bool SPI_Flash_writePage(uint32_t a, uint8_t *d) { CHECK((a % 256U) == 0U); CHECK(a >= SSTV_STORE_FLASH_ADDRESS && a < FIELD_DATA_FLASH_ADDRESS); for (int i = 0; i < 256; i++) flash[a + i] &= d[i]; return true; }

static uint16_t pic[SSTV_RX_ROWS * SSTV_RX_WIDTH];

static void fill(uint16_t v) { for (unsigned i = 0; i < SSTV_RX_ROWS * SSTV_RX_WIDTH; i++) pic[i] = (uint16_t)(v + i); }

static sstvStoreHeader_t header(unsigned slot) { sstvStoreHeader_t h; memcpy(&h, &flash[SSTV_STORE_FLASH_ADDRESS + slot * SSTV_STORE_SLOT_SIZE], sizeof h); return h; }

static void save(uint16_t v, uint32_t utc)
{
	fill(v);
	CHECK(sstvStoreBegin(pic, 120, "Martin 1", 14580000U, utc));
	CHECK(sstvStoreBegin(pic, 120, "x", 0, 0) == false); // busy
	int ticks = 0;
	while (!sstvStoreTick()) { ticks++; CHECK(ticks < 20); }
	CHECK(ticks == 10); // one sector per tick, header after the last one
	CHECK(sstvStoreBusy() == false);
}

int main(void)
{
	flash = malloc(FLASH_SIZE);
	memset(flash, 0x5A, FLASH_SIZE); // old GPS log bytes, not erased
	CHECK(sstvStoreCount() == 0U);

	// cut short: no valid slot
	fill(1);
	CHECK(sstvStoreBegin(pic, 120, "Robot 36", 1, 2));
	for (int i = 0; i < 5; i++) sstvStoreTick();
	CHECK(sstvStoreCount() == 0U);
	sstvStoreFinish();
	CHECK(sstvStoreCount() == 1U);
	CHECK(header(0).seq == 1U && strcmp(header(0).mode, "Robot 36") == 0);

	save(100, 1000);
	sstvStoreHeader_t h = header(1);
	CHECK(h.magic == SSTV_STORE_MAGIC && h.seq == 2U && h.utc == 1000U && h.rxFreq == 14580000U && h.rows == 120U && h.width == 160U && h.height == 120U);
	CHECK(memcmp(&flash[SSTV_STORE_FLASH_ADDRESS + SSTV_STORE_SLOT_SIZE + SSTV_STORE_PIXELS_OFFSET], pic, sizeof pic) == 0);
	CHECK(flash[SSTV_STORE_FLASH_ADDRESS + SSTV_STORE_SLOT_SIZE + 32] == 0xFF); // rest of the header page erased

	// ring: 8 slots, the 9th picture replaces the oldest
	for (unsigned i = 3; i <= 9; i++) save((uint16_t)(i * 7), i);
	CHECK(sstvStoreCount() == 8U);
	CHECK(header(0).seq == 9U); // slot 0 (seq 1) replaced
	CHECK(header(1).seq == 2U);
	CHECK(memcmp(&flash[SSTV_STORE_FLASH_ADDRESS + SSTV_STORE_PIXELS_OFFSET], pic, sizeof pic) == 0);
	save(5, 10);
	CHECK(header(1).seq == 10U);

	// flash error during a save: the slot gets no header
	fill(77);
	failErase = 4;
	CHECK(sstvStoreBegin(pic, 120, "PD 120", 0, 0));
	sstvStoreFinish();
	CHECK(header(2).magic != SSTV_STORE_MAGIC);
	CHECK(sstvStoreCount() == 7U);

	// nothing outside the store was touched
	CHECK(flash[SSTV_STORE_FLASH_ADDRESS - 1] == 0x5A && flash[FIELD_DATA_FLASH_ADDRESS] == 0x5A);
	CHECK(SSTV_STORE_FLASH_ADDRESS == 0xFA0000U);

	printf(fails ? "sstv store: %d FAILED\n" : "sstv store: all OK\n", fails);
	return fails ? 1 : 0;
}
