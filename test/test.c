#include "minctest.h"

#define ENABLE_SOUND 0
#define ENABLE_LCD 0
#include "../peanut_gb.h"

#include <assert.h>
#include <stdio.h>

struct priv
{
	char str[1024];
	unsigned int count;
};

/**
 * Return byte from blarrg test ROM.
 */
uint8_t gb_rom_read_cpu_instrs(struct gb_s *gb, const uint_fast32_t addr)
{
#include "cpu_instrs.h"
	assert(addr < cpu_instrs_gb_len);
	return cpu_instrs_gb[addr];
}

/**
 * Return byte from blarrg test ROM.
 */
uint8_t gb_rom_read_instr_timing(struct gb_s *gb, const uint_fast32_t addr)
{
#include "instr_timing.h"
	assert(addr < instr_timing_gb_len);
	return instr_timing_gb[addr];
}

/**
 * Ignore cart RAM writes, since the test doesn't require it.
 */
void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr, const uint8_t val)
{
	return;
}

/**
 * Ignore cart RAM reads, since the test doesn't require it.
 */
uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr)
{
	return 0xFF;
}

/**
 * Abort on any error.
 */
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t val)
{
	abort();
}

void gb_serial_tx(struct gb_s *gb, const uint8_t tx)
{
	struct priv *p = gb->direct.priv;
	char c = tx;

	/* Do not save any more characters if buffer does not have room for
	 * nul character. */
	if(p->count == (1023 - 1))
		return;

	/* Filter newlines to make test output cleaner. */
	if(tx < 32)
		c = ' ';

	printf("%c", c);
	p->str[p->count++] = c;
}

void test_cpu_inst(void)
{
	struct gb_s gb;
	const unsigned short pc_end = 0x06F1; /* Test ends when PC is this value. */
	struct priv p = { .count = 0 };
	enum gb_init_error_e gb_err;

	/* Run ROM test. */
	gb_err = gb_init(&gb, &gb_rom_read_cpu_instrs, &gb_cart_ram_read,
			&gb_cart_ram_write, &gb_error, &p);
	lok(gb_err == GB_INIT_NO_ERROR);
	if(gb_err != GB_INIT_NO_ERROR)
		return;

	gb_init_serial(&gb, &gb_serial_tx, NULL);

	printf("Serial: ");

	/* Step CPU until test is complete. */
	while(gb.cpu_reg.pc.reg != pc_end)
		__gb_step_cpu(&gb);

	p.str[p.count++] = '\0';

	/* Check test results. */
	lok(strstr(p.str, "Passed all tests") != NULL);

	return;
}

void test_instr_timing(void)
{
	struct gb_s gb;
	const unsigned short pc_end = 0xC8B0; /* Test ends when PC is this value. */
	struct priv p = { .count = 0 };
	enum gb_init_error_e gb_err;

	/* Run ROM test. */
	gb_err = gb_init(&gb, &gb_rom_read_instr_timing, &gb_cart_ram_read,
			&gb_cart_ram_write, &gb_error, &p);
	lok(gb_err == GB_INIT_NO_ERROR);
	if(gb_err != GB_INIT_NO_ERROR)
		return;

	gb_init_serial(&gb, &gb_serial_tx, NULL);

	printf("Serial: ");

	/* Step CPU until test is complete. */
	while(gb.cpu_reg.pc.reg != pc_end)
		__gb_step_cpu(&gb);

	p.str[p.count++] = '\0';

	/* Check test results. */
	lok(strstr(p.str, "Passed") != NULL);

	return;
}

int main(void)
{
	lrun("cpu_inst blarrg tests    ", test_cpu_inst);
	lrun("instr_timing blarrg tests", test_instr_timing);
	return lfails != 0;
}
