#define ENABLE_SOUND 0
#define ENABLE_LCD 0

/* Import emulator library. */
#include "../../peanut_gb.h"

#include "prof.h"

#include <stdio.h>
#include <time.h>

/**
 * Return byte from blarrg test ROM.
 */
uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr)
{
	/* Import blarrg CPU test ROM. */
#include "../../test/cpu_instrs.h"
	(void)gb;
	return cpu_instrs_gb[addr];
}

/**
 * Ignore cart RAM writes, since the test doesn't require it.
 */
void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr,
		const uint8_t val)
{
	(void)gb;
	(void)addr;
	(void)val;
	return;
}

/**
 * Ignore cart RAM reads, since the test doesn't require it.
 */
uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr)
{
	(void)gb;
	(void)addr;
	return 0xFF;
}

/**
 * Ignore all errors.
 */
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err,
		const uint16_t val)
{
	(void)gb;
	(void)gb_err;
	(void)val;
	return;
}

int main(void)
{
	const unsigned short pc_end = 0x06F1; /* Test ends when PC is this value. */
	struct gb_s gb;
	int ret;

	PROF_START();

	ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read,
			&gb_cart_ram_write, &gb_error, NULL);

	if(ret != GB_INIT_NO_ERROR)
	{
		printf("Error: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	/* Step CPU until test is complete. */
	while(gb.cpu_reg.pc != pc_end)
		gb_run_frame(&gb);

	PROF_STDOUT();

	return 0;
}
