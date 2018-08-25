#include <errno.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gameboy.h"

struct priv_t
{
	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
	/* Pointer to allocated memory holding save file. */
	uint8_t *save;
};

/**
 * Returns an byte from the ROM file at the given address.
 */
uint8_t gb_rom_read(uint32_t addr, struct gb_t *gb)
{
    struct priv_t *p = gb->priv;
    return p->rom[addr];
}

/**
 * Returns a pointer to the allocated space containing the ROM. Must be freed.
 */
uint8_t *read_rom_to_ram(const char *file_name)
{
	FILE *rom_file = fopen(file_name, "rb");
	long rom_size;
	uint8_t *rom = NULL;

	if(rom_file == NULL)
        goto err;

	fseek(rom_file, 0, SEEK_END);
	rom_size = ftell(rom_file);
	rewind(rom_file);
	rom = malloc(rom_size);

	if(fread(rom, sizeof(uint8_t), rom_size, rom_file) != rom_size)
        goto err;

	fclose(rom_file);
	return rom;

err:
	free(rom);
	fclose(rom_file);
	return NULL;
}

int main(int argc, char **argv)
{
    struct gb_t gb;
	struct priv_t priv;

	if(argc != 2)
	{
		printf("Usage: %s FILE\n", argv[0]);
		return EXIT_FAILURE;
	}

	if((priv.rom = read_rom_to_ram(argv[1])) == NULL)
	{
		printf("%s\n", strerror(errno));
		return EXIT_FAILURE;
	}

    /* TODO: Init GB */
    gb = gb_init(&gb_rom_read, &priv);

	/* TODO: Load Save File. */
	priv.save = malloc(gb_get_save_size(&gb));

	while(1)
	{
		/* TODO: Get joypad input. */
	}

	free(priv.rom);
	free(priv.save);
	
	return EXIT_SUCCESS;
}
