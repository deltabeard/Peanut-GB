#define ENABLE_SOUND 0
#define ENABLE_LCD 0
#include "../peanut_gb.h"

#include <assert.h>
#include <stdio.h>

struct priv
{
	uint8_t *rom;
	size_t rom_sz;
};

/**
 * Return byte from ROM.
 */
uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr)
{
	struct priv *p = gb->direct.priv;
	assert(addr < p->rom_sz);
	return p->rom[addr];
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
	char c = tx;

	/* Filter newlines to make test output cleaner. */
	//if(tx < 32)
	//	c = ' ';

	putchar(c);
}

/**
 * Returns a pointer to the allocated space containing the ROM. Must be freed.
 */
static uint8_t *read_rom_to_ram(const char *file_name, size_t *sz)
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
	*sz = rom_size;
	return rom;
}

int main(int argc, char *argv[])
{
	struct gb_s gb;
	struct priv p;
	int ret = EXIT_FAILURE;
	char *rom_file_name;
	unsigned long frames;

	if(argc != 3)
	{
		printf("Usage: %s ROM FRAMES\n", argv[0]);
		goto err;
	}

	rom_file_name = argv[1];
	frames = strtoul(argv[2], NULL, 10);

	/* Copy input ROM file to allocated memory. */
	if((p.rom = read_rom_to_ram(rom_file_name, &p.rom_sz)) == NULL)
	{
		perror("ROM read failed");
		goto err;
	}

	/* Initialise context. */
	ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read,
			&gb_cart_ram_write, &gb_error, &p);

	if(ret != GB_INIT_NO_ERROR)
	{
		fprintf(stderr, "Peanut-GB failed to initialise: %d\n",
				ret);
		free(p.rom);
		goto err;
	}

	gb_init_serial(&gb, gb_serial_tx, NULL);
#if ENABLE_LCD
	gb_init_lcd(&gb, &lcd_draw_line);
	// gb.direct.interlace = 1;
#endif

	do
	{
		/* Execute CPU cycles until the screen has to be
		 * redrawn. */
		gb_run_frame(&gb);
	}
	while(frames--);

	free(p.rom);
	putchar('\n');

	ret = EXIT_SUCCESS;
err:
	return ret;
}
