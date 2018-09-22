#define ENABLE_SOUND 0
#define ENABLE_LCD 0

/* Import emulator library. */
#include "../../gameboy.h"

#include "prof.h"

#include <stdio.h>
#include <time.h>

/**
 * Return byte from blarrg test ROM.
 */
uint8_t gb_rom_read(struct gb_t *gb, const uint32_t addr)
{
	/* Import blarrg CPU test ROM. */
#include "../../test/cpu_instrs.h"
	return cpu_instrs_gb[addr];
}

/**
 * Ignore cart RAM writes, since the test doesn't require it.
 */
void gb_cart_ram_write(struct gb_t *gb, const uint32_t addr, const uint8_t val)
{
	return;
}

/**
 * Ignore cart RAM reads, since the test doesn't require it.
 */
uint8_t gb_cart_ram_read(struct gb_t *gb, const uint32_t addr)
{
	return 0xFF;
}

/**
 * Ignore all errors.
 */
void gb_error(struct gb_t *gb, const enum gb_error_e gb_err, const uint16_t val)
{
	return;
}

uint8_t gb_serial_transfer(struct gb_t *gb, const uint8_t tx)
{
	return 0;
}

int main(void)
{
	const unsigned short pc_end = 0x06F1; /* Test ends when PC is this value. */
	struct gb_t gb;

	PROF_START();

	gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write,
			&gb_error, &gb_serial_transfer, NULL);

	/* Step CPU until test is complete. */
	while(gb.cpu_reg.pc != pc_end)
		gb_run_frame(&gb);

	PROF_STDOUT();

	return 0;
}
