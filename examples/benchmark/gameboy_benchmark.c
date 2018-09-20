#define ENABLE_SOUND 0
#define ENABLE_LCD 0

/* Import emulator library. */
#include "../../gameboy.h"

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

int main(void)
{
	unsigned long long benchmark_ticks_total = 0;
	unsigned long long benchmark_fps_total = 0;

	const unsigned short pc_end = 0x06F1; /* Test ends when PC is this value. */

	puts("Benchmark started");

	for(unsigned int i = 0; i < 5; i++)
	{
		/* Start benchmark. */
		struct gb_t gb;
		const unsigned long long start_time = (unsigned long)clock();
		unsigned long long bench_ticks;
		unsigned long long bench_fps;
		unsigned long long frames = 0;

		/* Initialise context. */
		gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write,
				&gb_error, NULL);

		/* Step CPU until test is complete. */
		while(gb.cpu_reg.pc != pc_end)
		{
			gb_run_frame(&gb);
			frames++;
		}

		/* End benchmark. */
		bench_ticks = (unsigned long long)clock() - start_time;
		benchmark_ticks_total += bench_ticks;

		bench_fps = frames / ((double)bench_ticks / CLOCKS_PER_SEC);
		benchmark_fps_total += bench_fps;

		printf("Benchmark %i: %lld\tFPS: %lld\n", i, bench_ticks, bench_fps);
	}

	printf("Average    : %lld\tFPS: %lld\n",
			benchmark_ticks_total / 5, benchmark_fps_total / 5);

	return 0;
}
