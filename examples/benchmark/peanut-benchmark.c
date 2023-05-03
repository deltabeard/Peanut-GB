/**
 * MIT License
 * Copyright (c) 2018-2023 Mahyar Koshkouei
 *
 * Performs a benchmark of Peanut-GB with a specified ROM.
 * Plays the ROM five times and prints the FPS for each play.
 */
#ifndef ENABLE_LCD
# define ENABLE_LCD 1
#endif

/* Sound is disabled for this project. */
#define ENABLE_SOUND 0

/* Import emulator library. */
#include "../../peanut_gb.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const uint_fast32_t frames_per_run = 64 * 1024;

struct priv_t
{
	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
	/* Pointer to allocated memory holding save file. */
	uint8_t *cart_ram;

	/* Frame buffer */
	uint16_t fb[LCD_HEIGHT][LCD_WIDTH];
};

/**
 * Returns a byte from the ROM file at the given address.
 */
static uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr)
{
	const struct priv_t * const p = gb->direct.priv;
	return p->rom[addr];
}

/**
 * Returns a byte from the cartridge RAM at the given address.
 */
static uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr)
{
	const struct priv_t * const p = gb->direct.priv;
	return p->cart_ram[addr];
}

/**
 * Writes a given byte to the cartridge RAM at the given address.
 */
static void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr,
		const uint8_t val)
{
	const struct priv_t * const p = gb->direct.priv;
	p->cart_ram[addr] = val;
}

/**
 * Returns a pointer to the allocated space containing the ROM. Must be freed.
 */
static uint8_t *read_rom_to_ram(const char *file_name)
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

/**
 * Ignore all errors.
 */
static void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t addr)
{
	const char* gb_err_str[GB_INVALID_MAX] = {
		"UNKNOWN",
		"INVALID OPCODE",
		"INVALID READ",
		"INVALID WRITE",
		"HALT FOREVER"
	};
	struct priv_t *priv = gb->direct.priv;

	fprintf(stderr, "Error %d occurred: %s at %04X\n. Exiting.\n",
			gb_err, gb_err_str[gb_err], addr);

	/* Free memory and then exit. */
	free(priv->cart_ram);
	free(priv->rom);
	exit(EXIT_FAILURE);
}

#if ENABLE_LCD
/**
 * Draws scanline into framebuffer.
 */
static void lcd_draw_line(struct gb_s *gb, const uint8_t pixels[160],
		const uint_least8_t line)
{
	struct priv_t *priv = gb->direct.priv;
	const uint16_t palette[] = { 0x7FFF, 0x5294, 0x294A, 0x0000 };

	for (unsigned int x = 0; x < LCD_WIDTH; x++)
		priv->fb[line][x] = palette[pixels[x] & 3];
}
#endif

int main(int argc, char **argv)
{
	/* Must be freed */
	char *rom_file_name = NULL;

	switch(argc)
	{
		case 2:
			rom_file_name = argv[1];
			break;

		default:
			fprintf(stderr, "%s ROM\n", argv[0]);
			exit(EXIT_FAILURE);
	}

	for(unsigned int i = 0; i < 5; i++)
	{
		/* Start benchmark. */
		struct gb_s gb;
		struct priv_t priv;

		clock_t start_time;
		uint_fast32_t frames = 0;
		enum gb_init_error_e ret;

		/* Copy input ROM file to allocated memory. */
		if((priv.rom = read_rom_to_ram(rom_file_name)) == NULL)
		{
			printf("%d: %s\n", __LINE__, strerror(errno));
			exit(EXIT_FAILURE);
		}

		/* Initialise context. */
		ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read,
				&gb_cart_ram_write, &gb_error, &priv);

		if(ret != GB_INIT_NO_ERROR)
		{
			fprintf(stderr, "Peanut-GB failed to initialise: %d\n",
					ret);
			exit(EXIT_FAILURE);
		}

		printf("Run %u: ", i);
		priv.cart_ram = malloc(gb_get_save_size(&gb));

#if ENABLE_LCD
		gb_init_lcd(&gb, &lcd_draw_line);
		// gb.direct.interlace = 1;
#endif

		start_time = clock();

		do
		{
			/* Execute CPU cycles until the screen has to be
			 * redrawn. */
			gb_run_frame(&gb);
		}
		while(++frames < frames_per_run);

		{
			double duration =
				(double)(clock() - start_time) / CLOCKS_PER_SEC;
			double fps = frames / duration;
			printf("%f FPS, dur: %f\n", fps, duration);
		}

		free(priv.cart_ram);
		free(priv.rom);
	}

	return EXIT_SUCCESS;
}
