#define ENABLE_SOUND 0
#define ENABLE_LCD 0

/* Import emulator library. */
#include "../../peanut_gb.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct priv_t
{
	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
};

/**
 * Return byte from blarrg test ROM.
 */
uint8_t gb_rom_read(struct gb_t *gb, const uint_fast32_t addr)
{
	const struct priv_t * const p = gb->direct.priv;
	return p->rom[addr];
}

/**
 * Ignore cart RAM writes, since the test doesn't require it.
 */
void gb_cart_ram_write(struct gb_t *gb, const uint_fast32_t addr, const uint8_t val)
{
	(void)gb;
	(void)addr;
	(void)val;
	return;
}

/**
 * Ignore cart RAM reads, since the test doesn't require it.
 */
uint8_t gb_cart_ram_read(struct gb_t *gb, const uint_fast32_t addr)
{
	(void)gb;
	(void)addr;
	return 0xFF;
}

/**
 * Ignore all errors.
 */
void gb_error(struct gb_t *gb, const enum gb_error_e gb_err, const uint16_t val)
{
	(void)gb;
	(void)gb_err;
	(void)val;
	return;
}

/**
 * Returns a pointer to the allocated space containing the ROM. Must be freed.
 */
uint8_t *read_rom_to_ram(const char *file_name)
{
	FILE *rom_file = fopen(file_name, "rb");
	size_t rom_size;
	uint8_t *rom = NULL;

	if(rom_file == NULL)
		return NULL;

	fseek(rom_file, 0, SEEK_END);
	rom_size = ftell(rom_file);
	rewind(rom_file);
	rom = malloc(rom_size);

	if(fread(rom, sizeof(uint8_t), rom_size, rom_file) != rom_size)
	{
		free(rom);
		fclose(rom_file);
		return NULL;
	}

	fclose(rom_file);
	return rom;
}

int main(int argc, char **argv)
{
	uint_fast32_t benchmark_ticks_total = 0;
	uint_fast32_t benchmark_fps_total = 0;
	struct priv_t priv = { .rom = NULL };

	if(argc != 2)
	{
		printf("%s ROM\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Copy input ROM file to allocated memory. */
	if((priv.rom = read_rom_to_ram(argv[1])) == NULL)
	{
		printf("Error(%d): %s\n", __LINE__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	puts("Benchmark started");

	for(unsigned int i = 0; i < 5; i++)
	{
		/* Start benchmark. */
		struct gb_t gb;
		const uint_fast64_t start_time = (uint_fast64_t)clock();
		uint_fast64_t bench_ticks;
		uint_fast64_t bench_fps;
		uint_fast64_t frames_to_run = 60 * 60 * 2; // 2 Minutes worth of frames
		const uint_fast64_t frames = frames_to_run;
		int ret;

		/* Initialise context. */
		ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read,
				&gb_cart_ram_write, &gb_error, &priv);

		if(ret != GB_INIT_NO_ERROR)
		{
			printf("Error(%d): %d\n", __LINE__, ret);
			exit(EXIT_FAILURE);
		}

		/* Step CPU until test is complete. */
		do
		{
			gb_run_frame(&gb);
		} while(--frames_to_run);

		/* End benchmark. */
		bench_ticks = (uint_fast64_t)clock() - start_time;
		benchmark_ticks_total += bench_ticks;

		bench_fps = frames / ((double)bench_ticks / CLOCKS_PER_SEC);
		benchmark_fps_total += bench_fps;

		printf("Benchmark %i: %ld\tFPS: %ld\n", i, bench_ticks, bench_fps);
	}

	printf("Average    : %ld\tFPS: %ld\n",
			benchmark_ticks_total / 5, benchmark_fps_total / 5);

	return 0;
}
