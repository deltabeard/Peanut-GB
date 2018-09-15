#include "minctest.h"
#include "cpu_instrs.h"

#define ENABLE_SOUND 0
#define ENABLE_LCD 0

#include "../gameboy.h"

#include <string.h>
#include <stdlib.h>
#include <time.h>

/**
 * Return byte from blarrg test ROM.
 */
uint8_t gb_rom_read(struct gb_t **gb, const uint32_t addr)
{
	return cpu_instrs_gb[addr];
}

/**
 * Ignore cart RAM writes, since the test doesn't require it.
 */
void gb_cart_ram_write(struct gb_t **gb, const uint32_t addr, const uint8_t val)
{
	return;
}

/**
 * Ignore cart RAM reads, since the test doesn't require it.
 */
uint8_t gb_cart_ram_read(struct gb_t **gb, const uint32_t addr)
{
	return 0xFF;
}

/**
 * Ignore all errors.
 */
void gb_error(struct gb_t **gb, const enum gb_error_e gb_err)
{
	return;
}

void test_cpu_inst(void)
{
	struct gb_t gb;
	struct gb_t *p = &gb;
	char str[1024];
	unsigned int count = 0;
	const unsigned short pc_end = 0x06F1; /* Test ends when PC is this value. */

	/* Run ROM test. */
    gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write, &gb_error,
			NULL);

	printf("Serial: ");

	/* Step CPU until test is complete. */
	while(gb.cpu_reg.pc != pc_end)
	{
		__gb_step_cpu(&p);

		/* Detect serial transmission. Test status is pushed to serial by the
		 * test ROM. */
		if(gb.gb_reg.SC == 0x81)
		{
			printf("%c", gb.gb_reg.SB != '\n' ? gb.gb_reg.SB : ' ');
			str[count++] = gb.gb_reg.SB;
			if(count == 1024)
				abort();
			
			/* Simulate serial read, as emulator does not support serial
			 * transmission yet. */
			gb.gb_reg.SC = 0x01;
		}
	}

	/* Check test results. */
	lok(strstr(str, "Passed all tests") != NULL);

	return;
}

int main(void)
{
	lrun("cpu_inst blarrg tests", test_cpu_inst);
	return lfails != 0;
}