#define ENABLE_LCD 1

#include "fenster.h"
#include "../../peanut_gb.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct priv_t
{
	/* Fenster context. */
	struct fenster f;
	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
	/* Pointer to allocated memory holding save file. */
	uint8_t *cart_ram;
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
 * Handles an error reported by the emulator. The emulator context may be used
 * to better understand why the error given in gb_err was reported.
 */
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t addr)
{
	const char* gb_err_str[GB_INVALID_MAX] = {
		"UNKNOWN",
		"INVALID OPCODE",
		"INVALID READ",
		"INVALID WRITE",
		"HALT FOREVER"
	};
	struct priv_t *priv = gb->direct.priv;

	/* Record save file. */
	//write_cart_ram_file("recovery.sav", &priv->cart_ram, gb_get_save_size(gb));

	fprintf(stderr, "Error %d occurred: %s at %04X\n. "
		"Cart RAM saved to recovery.sav\n"
		"Exiting.\n",
		gb_err, gb_err_str[gb_err], addr);

	/* Free memory and then exit. */
	free(priv->cart_ram);
	free(priv->rom);
	exit(EXIT_FAILURE);
}

/**
 * Draws scanline into framebuffer.
 */
void lcd_draw_line(struct gb_s *gb, const uint8_t pixels[160],
		   const uint_least8_t line)
{
	struct priv_t *priv = gb->direct.priv;
	const uint32_t palette[4] = { 0xFFFFFF, 0xFF0000, 0x00FF00, 0 };

	for(unsigned int x = 0; x < LCD_WIDTH; x++)
	{
		fenster_pixel(&priv->f, x, line) = palette[pixels[x] & 3];
	}
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
	if(rom == NULL)
		goto out;

	if(fread(rom, sizeof(uint8_t), rom_size, rom_file) != rom_size)
	{
		free(rom);
		rom = NULL;
		goto out;
	}

out:
	fclose(rom_file);
	return rom;
}

int read_cart_ram_file(const char *save_file_name, uint8_t **dest,
			const size_t len)
{
	FILE *f;

	/* If save file not required. */
	if(len == 0)
	{
		*dest = NULL;
		return 0;
	}

	/* Allocate enough memory to hold save file. */
	if((*dest = malloc(len)) == NULL)
	{
		return -1;
		//printf("%d: %s\n", __LINE__, strerror(errno));
		//exit(EXIT_FAILURE);
	}

	f = fopen(save_file_name, "rb");

	/* It doesn't matter if the save file doesn't exist. We initialise the
	 * save memory allocated above. The save file will be created on exit. */
	if(f == NULL)
	{
		memset(*dest, 0, len);
		return 0;
	}

	/* Read save file to allocated memory. */
	fread(*dest, sizeof(uint8_t), len, f);
	fclose(f);

	return 0;
}

int write_cart_ram_file(const char *save_file_name, uint8_t **dest,
			 const size_t len)
{
	FILE *f;

	if(len == 0 || *dest == NULL)
		return 0;

	if((f = fopen(save_file_name, "wb")) == NULL)
	{
		return -1;
		//puts("Unable to open save file.");
		//printf("%d: %s\n", __LINE__, strerror(errno));
		//exit(EXIT_FAILURE);
	}

	/* Record save file. */
	fwrite(*dest, sizeof(uint8_t), len, f);
	fclose(f);

	return 0;
}


int main(int argc, char *argv[])
{
	static uint32_t fbuf[LCD_WIDTH * LCD_HEIGHT];
	static char title[128] = "Peanut-Fenster: ";
	struct gb_s gb;
	struct priv_t priv =
	{
		.f =
		{
			.title = title,
			.width = LCD_WIDTH,
			.height = LCD_HEIGHT,
			.buf = fbuf
		},
		.rom = NULL,
		.cart_ram = NULL
	};
	char *rom_file_name = NULL;
	char *save_file_name = NULL;
	int ret = EXIT_FAILURE;
	const double target_speed_ms = 1000.0 / VERTICAL_SYNC;
	enum gb_init_error_e gb_ret;
	int64_t tim_old, tim_new;

	if(argc == 2)
	{
		rom_file_name = argv[1];
	}
	else if(argc == 3)
	{
		rom_file_name = argv[1];
		save_file_name = argv[2];
	}
	else
	{
		fprintf(stderr, "peanut_fenster ROM.GB [ROM.SAV]\n");
		return EXIT_FAILURE;
	}

	/* Copy input ROM file to allocated memory. */
	if((priv.rom = read_rom_to_ram(rom_file_name)) == NULL)
	{
		printf("%d: %s\n", __LINE__, strerror(errno));
		ret = EXIT_FAILURE;
		goto out;
	}


	/* Initialise emulator context. */
	gb_ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write,
			 &gb_error, &priv);

	switch(gb_ret)
	{
	case GB_INIT_NO_ERROR:
		break;

	case GB_INIT_CARTRIDGE_UNSUPPORTED:
		puts("Unsupported cartridge.");
		goto out;

	case GB_INIT_INVALID_CHECKSUM:
		puts("Invalid ROM: Checksum failure.");
		goto out;

	default:
		printf("Unknown error: %d\n", gb_ret);
		goto out;
	}

	/* Load Save File. */
	if(gb_get_save_size(&gb) != 0)
	{

		/* If no save file is specified, copy save file (with specific name) to
		 * allocated memory. */
		if(save_file_name == NULL)
		{
			char *str_replace;
			const char extension[] = ".sav";

			/* Allocate enough space for the ROM file name, for the "sav"
			 * extension and for the null terminator. */
			save_file_name = malloc(strlen(rom_file_name) + strlen(extension) + 1);

			if(save_file_name == NULL)
			{
				fprintf(stderr, "%d: %s\n", __LINE__, strerror(errno));
				return EXIT_FAILURE;
			}

			/* Copy the ROM file name to allocated space. */
			strcpy(save_file_name, rom_file_name);

			/* If the file name does not have a dot, or the only dot is at
			 * the start of the file name, set the pointer to begin
			 * replacing the string to the end of the file name, otherwise
			 * set it to the dot. */
			if((str_replace = strrchr(save_file_name, '.')) == NULL ||
					str_replace == save_file_name)
				str_replace = save_file_name + strlen(save_file_name);

			/* Copy extension to string including terminating null byte. */
			for(unsigned int i = 0; i <= strlen(extension); i++)
				*(str_replace++) = extension[i];
		}

		read_cart_ram_file(save_file_name, &priv.cart_ram, gb_get_save_size(&gb));
	}

	/* Set the RTC of the game cartridge. Only used by games that support it. */
	{
		time_t rawtime;
		time(&rawtime);
#ifdef _POSIX_C_SOURCE
		struct tm timeinfo;
		localtime_r(&rawtime, &timeinfo);
#else
		struct tm *timeinfo;
		timeinfo = localtime(&rawtime);
#endif

		/* You could potentially force the game to allow the player to
		 * reset the time by setting the RTC to invalid values.
		 *
		 * Using memset(&gb->cart_rtc, 0xFF, sizeof(gb->cart_rtc)) for
		 * example causes Pokemon Gold/Silver to say "TIME NOT SET",
		 * allowing the player to set the time without having some dumb
		 * password.
		 *
		 * The memset has to be done directly to gb->cart_rtc because
		 * gb_set_rtc() processes the input values, which may cause
		 * games to not detect invalid values.
		 */

		/* Set RTC. Only games that specify support for RTC will use
		 * these values. */
#ifdef _POSIX_C_SOURCE
		gb_set_rtc(&gb, &timeinfo);
#else
		gb_set_rtc(&gb, timeinfo);
#endif
	}

#if ENABLE_LCD
	gb_init_lcd(&gb, &lcd_draw_line);
#endif
	{
		const size_t title_len = strlen(title);
		printf("ROM: %s\n", gb_get_rom_name(&gb, title + title_len));
		printf("MBC: %d\n", gb.mbc);
	}

	if(fenster_open(&priv.f) != 0)
		goto out;

	tim_old = fenster_time();
	while(fenster_loop(&priv.f) == 0)
	{
		int64_t delay;

		/* Execute CPU cycles until the screen has to be redrawn. */
		gb_run_frame(&gb);

		tim_new = fenster_time();
		delay = (int64_t)target_speed_ms - (tim_new - tim_old);
		if(delay > 0)
			fenster_sleep(delay);

		tim_old = fenster_time();
	}

	fenster_close(&priv.f);
	free(priv.rom);
	free(priv.cart_ram);
	if(argc == 2)
		free(save_file_name);

	ret = EXIT_SUCCESS;
out:
	return ret;
}
