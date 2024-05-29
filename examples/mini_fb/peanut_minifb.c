#define ENABLE_SOUND 0
#define ENABLE_LCD 1

/* Import emulator library. */
#include "../../peanut_gb.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "MiniFB.h"

struct priv_t
{
	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
	/* Pointer to allocated memory holding save file. */
	uint8_t *cart_ram;

	/* Frame buffer */
	uint32_t fb[LCD_HEIGHT][LCD_WIDTH];
};

/**
 * Returns a byte from the ROM file at the given address.
 */
uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr)
{
	const struct priv_t * const p = gb->direct.priv;
	return p->rom[addr];
}

/**
 * Returns a byte from the cartridge RAM at the given address.
 */
uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr)
{
	const struct priv_t * const p = gb->direct.priv;
	return p->cart_ram[addr];
}

/**
 * Writes a given byte to the cartridge RAM at the given address.
 */
void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr,
		       const uint8_t val)
{
	const struct priv_t * const p = gb->direct.priv;
	p->cart_ram[addr] = val;
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

/**
 * Ignore all errors.
 */
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t val)
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
			gb_err, gb_err_str[gb_err], val);

	/* Free memory and then exit. */
	free(priv->cart_ram);
	free(priv->rom);
	exit(EXIT_FAILURE);
}

#if ENABLE_LCD
/**
 * Draws scanline into framebuffer.
 */
void lcd_draw_line(struct gb_s *gb, const uint8_t pixels[160],
		   const uint_least8_t line)
{
	struct priv_t *priv = gb->direct.priv;
	const uint32_t palette[] = { 0xFFFFFF, 0xA5A5A5, 0x525252, 0x000000 };

	for(unsigned int x = 0; x < LCD_WIDTH; x++)
		priv->fb[line][x] = palette[pixels[x] & 3];
}
#endif

int main(int argc, char **argv)
{
	/* Must be freed */
	char *rom_file_name = NULL;
	static struct gb_s gb;
	static struct priv_t priv;
	enum gb_init_error_e ret;

	switch(argc)
	{
	case 2:
		rom_file_name = argv[1];
		break;

	default:
		fprintf(stderr, "%s ROM\n", argv[0]);
		exit(EXIT_FAILURE);
	}

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
		printf("Error: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	priv.cart_ram = malloc(gb_get_save_size(&gb));

#if ENABLE_LCD
	gb_init_lcd(&gb, &lcd_draw_line);
	// gb.direct.interlace = 1;
#endif

	if(!mfb_open("Peanut-minifb", LCD_WIDTH, LCD_HEIGHT))
		return EXIT_FAILURE;

	while(1)
	{
		const double target_speed_us = 1000000.0 / VERTICAL_SYNC;
		int_fast16_t delay;
		unsigned long start, end;
		struct timeval timecheck;
		int state;

		gettimeofday(&timecheck, NULL);
		start = (long)timecheck.tv_sec * 1000000 +
			(long)timecheck.tv_usec;

		/* Execute CPU cycles until the screen has to be redrawn. */
		gb_run_frame(&gb);

		state = mfb_update(priv.fb);

		/* ESC pressed */
		if(state < 0)
			break;

		gettimeofday(&timecheck, NULL);
		end = (long)timecheck.tv_sec * 1000000 +
		      (long)timecheck.tv_usec;

		delay = target_speed_us - (end - start);

		/* If it took more than the maximum allowed time to draw frame,
		 * do not delay.
		 * Interlaced mode could be enabled here to help speed up
		 * drawing.
		 */
		if(delay < 0)
			continue;

		usleep(delay);
	}

	mfb_close();
	free(priv.cart_ram);
	free(priv.rom);

	return EXIT_SUCCESS;
}
