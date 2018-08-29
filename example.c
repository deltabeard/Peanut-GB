#include <errno.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>
#include <SDL/SDL_main.h>

#include "gameboy.h"

struct priv_t
{
	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
	/* Pointer to allocated memory holding save file. */
	uint8_t *cart_ram;
};

/**
 * Returns an byte from the ROM file at the given address.
 */
uint8_t gb_rom_read(const struct gb_t * const gb, const uint32_t addr)
{
    const struct priv_t * const p = gb->priv;
    return p->rom[addr];
}

uint8_t gb_cart_ram_read(const struct gb_t * const gb, const uint32_t addr)
{
	const struct priv_t * const p = gb->priv;
	return p->cart_ram[addr];
}

void gb_cart_ram_write(struct gb_t * const gb, const uint32_t addr,
	const uint8_t val)
{
	struct priv_t * const p = gb->priv;
	p->cart_ram[addr] = val;
}

void gb_error(struct gb_t *gb, const enum gb_error_e gb_err)
{
	switch(gb_err)
	{
		case GB_INVALID_OPCODE:
			printf("Invalid opcode %#04x at PC: %#06x, SP: %#06x\n",
					__gb_read(gb, gb->cpu_reg.pc),
					gb->cpu_reg.pc, gb->cpu_reg.sp);
			break;
	}
	
	abort();
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
	const unsigned int height = 144;
	const unsigned int width = 160;
	unsigned int running = 1;
	SDL_Surface* screen;

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

	/* TODO: Sanity check input GB file. */
    /* TODO: Init GB */
    gb = gb_init(&gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write, &gb_error,
			&priv);

	/* TODO: Load Save File. */
	priv.cart_ram = malloc(gb_get_save_size(&gb));

	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(width, height, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	SDL_WM_SetCaption("DMG Emulator", 0);

	uint32_t fb[height][width];

	while(running)
	{
		uint32_t palette[4] = {0xFFFFFFFF, 0x99999999, 0x44444444, 0x00000000};
		uint32_t *screen_copy;
		SDL_Event event;
		
		while(SDL_PollEvent(&event))
		{
			if(event.type == SDL_QUIT)
				running = 0;
		}

		/* TODO: Get joypad input. */
		gb_run_frame(&gb);

		for (unsigned int y = 0; y < height; y++)
		{
			for (unsigned int x = 0; x < width; x++)
				fb[y][x] = palette[gb.gb_fb[y][x] & 3];
		}

		SDL_LockSurface(screen);
		screen_copy = (uint32_t *) screen->pixels;
		for(unsigned int y = 0; y < height; y++)
		{
			for (unsigned int x = 0; x < width; x++)
				*(screen_copy + x) = fb[y][x];
			
			screen_copy += screen->pitch / 4;
		}
		SDL_UnlockSurface(screen);
		SDL_Flip(screen);
		SDL_Delay(16);
	}

	SDL_Quit();
	free(priv.rom);
	free(priv.cart_ram);
	
	return EXIT_SUCCESS;
}
