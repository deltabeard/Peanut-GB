#include <stdio.h>
#include <stdint.h>

#include "gb.h"

/**
 * Returns a byte from the ROM file at the given address.
 */
static uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr)
{
	const struct priv * const p = gb->direct.priv;
	return p->rom[addr];
}

/**
 * Returns a byte from the cartridge RAM at the given address.
 */
static uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr)
{
	const struct priv * const p = gb->direct.priv;
	return p->cart_ram[addr];
}

/**
 * Writes a given byte to the cartridge RAM at the given address.
 */
static void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr,
		       const uint8_t val)
{
	const struct priv * const p = gb->direct.priv;
	p->cart_ram[addr] = val;
}


/**
 * Returns a pointer to the allocated space containing the ROM. Must be freed.
 */
static uint8_t *alloc_file(const char *file_name, size_t *file_len)
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
	if(rom == NULL) {
		fclose(rom_file);
		return NULL;
	}

	if(fread(rom, sizeof(uint8_t), rom_size, rom_file) != rom_size)
	{
		free(rom);
		fclose(rom_file);
		return NULL;
	}

	if(file_len != NULL)
		*file_len = rom_size;

	fclose(rom_file);
	return rom;
}

static uint8_t *read_cart_ram_file(const char *save_file_name, size_t len)
{
	uint8_t *ret;
	size_t read_size;

	ret = alloc_file(save_file_name, &read_size);

	if(len != read_size)
	{
		free(ret);
		return NULL;
	}

	/* It doesn't matter if the save file doesn't exist. We initialise the
	 * save memory allocated above. The save file will be created on exit. */
	if(ret == NULL)
		ret = calloc(len, 1);

	return ret;
}

static void write_cart_ram_file(const char *save_file_name, uint8_t **dest,
			 const size_t len)
{
	FILE *f;

	if(len == 0 || *dest == NULL)
		return;

	if((f = fopen(save_file_name, "wb")) == NULL)
	{
		return;
	}

	/* Record save file. */
	fwrite(f, *dest, sizeof(uint8_t), len);
	fclose(f);

	return;
}

/**
 * Handles an error reported by the emulator. The emulator context may be used
 * to better understand why the error given in gb_err was reported.
 */
static void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t addr)
{
	exit(EXIT_FAILURE);
}

#if ENABLE_LCD
/**
 * Draws scanline into framebuffer.
 */
static void lcd_draw_line(struct gb_s *gb, const uint8_t pixels[160],
		   const uint_least8_t line)
{
	struct priv *priv = gb->direct.priv;
	memcpy(&priv->fb[line][0], pixels, 160);
}
#endif

int gb_init_file(struct priv *p, const char *rom_file_name)
{
	struct gb_s *gb;

	if(p == NULL) return -1;
	if(rom_file_name == NULL) return -1;

	/* Copy input ROM file to allocated memory. */
	if((p->rom = alloc_file(rom_file_name, NULL)) == NULL)
	{
		fprintf(stderr, "%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}

	gb = &p->gb;

	/* Initialise context. */
	int ret = gb_init(gb, &gb_rom_read, &gb_cart_ram_read,
			&gb_cart_ram_write, &gb_error, p);
	if(ret != GB_INIT_NO_ERROR)
	{
		fprintf(stderr, "Peanut-GB failed to initialise: %d\n", ret);
		return -1;
	}

	p->cart_ram = malloc(gb_get_save_size(gb));

#if ENABLE_LCD
	gb_init_lcd(gb, &lcd_draw_line);
	// gb.direct.interlace = 1;
#endif

	return 0;
}

void gb_exit(struct priv *p)
{
	if(p == NULL) return;
	free(p->cart_ram);
	free(p->rom);
}