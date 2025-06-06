#include "minctest.h"

#define ENABLE_SOUND 0
#define ENABLE_LCD 1
#include "../peanut_gb.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dmg-acid2.gb.h" /* Generated via `xxd -i` */

struct priv
{
	char str[1024];
	unsigned int count;
};

/* Frame buffer storage for the dmg-acid2 test. */
struct acid_priv
{
	uint8_t fb[LCD_HEIGHT][LCD_WIDTH];
};

/* Update DMG_ACID2_HASH with the value printed when the LCD output is correct. */
#define DMG_ACID2_HASH 0x00000000u

/* FNV-1a 32-bit hashing function used to check the LCD output. */
static uint32_t fnv1a_hash(const void *data, size_t len)
{
	const uint8_t *bytes = data;
	/* 2166136261 is the standard 32‑bit FNV offset basis. */
	uint32_t hash = 2166136261u;

	while(len--)
	{
	        hash ^= *bytes++;
	        /* 16777619 (0x01000193) is the 32‑bit FNV prime. Decimal is
	         * used instead of hexadecimal for maximum portability. */
	        hash *= 16777619u;
	}

	return hash;
}

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

uint8_t gb_rom_read_acid(struct gb_s *gb, const uint_fast32_t addr)
{
	(void)gb;
	assert(addr < dmg_acid2_gb_len);
	return dmg_acid2_gb[addr];
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

static void acid_lcd_draw_line(struct gb_s *gb, const uint8_t *pixels,
                               const uint_fast8_t line)
{
	struct acid_priv *p = gb->direct.priv;
	memcpy(p->fb[line], pixels, LCD_WIDTH);
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

void test_dmg_acid2(void)
{
        struct gb_s gb;
	struct acid_priv p = {0};
	enum gb_init_error_e gb_err;

	gb_err = gb_init(&gb, &gb_rom_read_acid, &gb_cart_ram_read,
	                &gb_cart_ram_write, &gb_error, &p);
	lok(gb_err == GB_INIT_NO_ERROR);
	if(gb_err != GB_INIT_NO_ERROR)
	        return;

	gb_init_lcd(&gb, acid_lcd_draw_line);

	for(unsigned int i = 0; i < 100; i++)
	        gb_run_frame(&gb);

	{
	        uint32_t hash = fnv1a_hash(&p.fb[0][0],
	                                 LCD_WIDTH * LCD_HEIGHT);
	        if(hash != DMG_ACID2_HASH)
	                printf("dmg-acid2 LCD hash: 0x%08X\n", hash);
	        lok(hash == DMG_ACID2_HASH);
	}
}

int main(void)
{
	lrun("cpu_inst blarrg tests    ", test_cpu_inst);
	lrun("instr_timing blarrg tests", test_instr_timing);
	lrun("dmg-acid2 lcd test     ", test_dmg_acid2);
	return lfails != 0;
}
