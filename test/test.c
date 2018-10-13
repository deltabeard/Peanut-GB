#include "minctest.h"

#define ENABLE_SOUND 0
#define ENABLE_LCD 0
#include "../peanut_gb.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

struct priv
{
	char str[1024];
	unsigned int count;
};

/**
 * Return byte from blarrg test ROM.
 */
uint8_t gb_rom_read_cpu_instrs(struct gb_t *gb, const uint32_t addr)
{
#include "cpu_instrs.h"
	assert(addr < cpu_instrs_gb_len);
	return cpu_instrs_gb[addr];
}

/**
 * Return byte from blarrg test ROM.
 */
uint8_t gb_rom_read_instr_timing(struct gb_t *gb, const uint32_t addr)
{
#include "instr_timing.h"
	assert(addr < instr_timing_gb_len);
	return instr_timing_gb[addr];
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
	struct priv *p = gb->priv;

	/* Filter newlines to make test output cleaner. */
	if(tx < 32)
		return 0xFF;

	printf("%c", tx);
	p->str[p->count++] = tx;

	if(p->count == 1024)
		abort();

	/* No 2nd player connected. */
	return 0xFF;
}

void test_cpu_inst(void)
{
	struct gb_t gb;
	const unsigned short pc_end = 0x06F1; /* Test ends when PC is this value. */
	struct priv p = { .count = 0 };

	/* Run ROM test. */
	gb_init(&gb, &gb_rom_read_cpu_instrs, &gb_cart_ram_read,
			&gb_cart_ram_write, &gb_error, &gb_serial_transfer, &p);

	printf("Serial: ");

	/* Step CPU until test is complete. */
	while(gb.cpu_reg.pc != pc_end)
		__gb_step_cpu(&gb);

	p.str[p.count++] = '\0';

	/* Check test results. */
	lok(strstr(p.str, "Passed all tests") != NULL);

	return;
}

void test_instr_timing(void)
{
	struct gb_t gb;
	const unsigned short pc_end = 0xC8B0; /* Test ends when PC is this value. */
	struct priv p = { .count = 0 };

	/* Run ROM test. */
	gb_init(&gb, &gb_rom_read_instr_timing, &gb_cart_ram_read,
			&gb_cart_ram_write, &gb_error, &gb_serial_transfer, &p);

	printf("Serial: ");

	/* Step CPU until test is complete. */
	while(gb.cpu_reg.pc != pc_end)
		__gb_step_cpu(&gb);

	p.str[p.count++] = '\0';

	/* Check test results. */
	lok(strstr(p.str, "Passed") != NULL);

	return;
}

int main(void)
{
	lrun("cpu_inst blarrg tests", test_cpu_inst);
	lrun("instr_timing blarrg tests", test_instr_timing);
	return lfails != 0;
}
