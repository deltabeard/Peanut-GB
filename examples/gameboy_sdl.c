/**
 * MIT License
 * Copyright (c) 2018 Mahyar Koshkouei
 */

#include <errno.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL/SDL.h>
#include <SDL/SDL_main.h>

#include "../gameboy.h"

struct priv_t
{
	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
	/* Pointer to allocated memory holding save file. */
	uint8_t *cart_ram;
};

/**
 * Returns a byte from the ROM file at the given address.
 */
uint8_t gb_rom_read(struct gb_t *gb, const uint32_t addr)
{
    const struct priv_t * const p = gb->priv;
    return p->rom[addr];
}

/**
 * Returns a byte from the cartridge RAM at the given address.
 */
uint8_t gb_cart_ram_read(struct gb_t *gb, const uint32_t addr)
{
	const struct priv_t * const p = gb->priv;
	return p->cart_ram[addr];
}

/**
 * Writes a given byte to the cartridge RAM at the given address.
 */
void gb_cart_ram_write(struct gb_t *gb, const uint32_t addr,
	const uint8_t val)
{
	const struct priv_t * const p = gb->priv;
	p->cart_ram[addr] = val;
}

/**
 * Handles an error reported by the emulator. The emulator context may be used
 * to better understand why the error given in gb_err was reported.
 */
void gb_error(struct gb_t *gb, const enum gb_error_e gb_err)
{
	struct priv_t *priv = gb->priv;

	switch(gb_err)
	{
		case GB_INVALID_OPCODE:
			printf("Invalid opcode %#04x", __gb_read(gb, gb->cpu_reg.pc));
			break;

		case GB_INVALID_WRITE:
		case GB_INVALID_READ:
			return;
			printf("Invalid write");
			break;

		default:
			printf("Unknown error");
			break;
	}

	printf(" at PC: %#06x, SP: %#06x\n", gb->cpu_reg.pc, gb->cpu_reg.sp);

	puts("Press q to exit, or any other key to continue.");
	if(getchar() == 'q')
	{
		free(priv->rom);
		free(priv->cart_ram);
		exit(EXIT_FAILURE);
	}

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

void read_cart_ram_file(const char *save_file_name, uint8_t **dest,
	const size_t len)
{
	FILE *f;

	/* If save file not required. */
	if(len == 0)
	{
		*dest = NULL;
		return;
	}

	/* Allocate enough memory to hold save file. */
	if((*dest = malloc(len)) == NULL)
	{
		printf("%d: %s\n", __LINE__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	f = fopen(save_file_name, "rb");

	/* It doesn't matter if the save file doesn't exist. We initialise the
	 * save memory allocated above. The save file will be created on exit. */
	if(f == NULL)
	{
		memset(*dest, 0xFF, len);
		return;
	}

	/* Read save file to allocated memory. */
	fread(*dest, sizeof(uint8_t), len, f);
	fclose(f);
}

void write_cart_ram_file(const char *save_file_name, uint8_t **dest,
	const size_t len)
{
	FILE *f;

	if(len == 0 || *dest == NULL)
		return;

	if((f = fopen(save_file_name, "wb")) == NULL)
	{
		puts("Unable to open save file.");
		printf("%d: %s\n", __LINE__, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Record save file. */
	fwrite(*dest, sizeof(uint8_t), len, f);
	fclose(f);
}

int main(int argc, char **argv)
{
    struct gb_t gb;
	struct priv_t priv;
	const unsigned int height = 144;
	const unsigned int width = 160;
	unsigned int running = 1;
	SDL_Surface* screen;
	SDL_Event event;
	uint32_t fb[height][width];
	uint32_t new_ticks, old_ticks;
	char *save_file_name;

	/* Make sure a file name is given. */
	if(argc != 2)
	{
		printf("Usage: %s FILE\n", argv[0]);
		return EXIT_FAILURE;
	}

	/* Copy input ROM file to allocated memory. */
	if((priv.rom = read_rom_to_ram(argv[1])) == NULL)
	{
		printf("%d: %s\n", __LINE__, strerror(errno));
		return EXIT_FAILURE;
	}

	/* Copy save file (with specific name) to allocated memory. */
	{
		char *str_replace;
		char extension[] = "sav";
		save_file_name = malloc(strlen(argv[1]) + strlen(extension) + 1);

		/* Generate name of save file. */
		if((save_file_name = malloc(strlen(argv[1]) + 1)) == NULL)
		{
			printf("%d: %s\n", __LINE__, strerror(errno));
			return EXIT_FAILURE;
		}

		strcpy(save_file_name, argv[1]);
		str_replace = strrchr(save_file_name, '.');

		/* Copy extension to string including terminating null byte. */
		for(unsigned int i = 0; i < strlen(extension) + 1; i++)
			*(++str_replace) = extension[i];
	}
	
	/* TODO: Sanity check input GB file. */

    /* Initialise emulator context. */
    gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write, &gb_error,
			&priv);

	/* Load Save File. */
	read_cart_ram_file(save_file_name, &priv.cart_ram, gb_get_save_size(&gb));
	
	/* Initialise frontend implementation, in this case, SDL. */
	SDL_Init(SDL_INIT_VIDEO);
	screen = SDL_SetVideoMode(width, height, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	SDL_WM_SetCaption("DMG Emulator", 0);

	new_ticks = SDL_GetTicks();

	while(running)
	{
		const uint32_t palette[4] = {
			0xFFFFFFFF, 0x99999999, 0x44444444, 0x00000000
		};
		uint32_t *screen_copy;
		int32_t delay;
		
		/* TODO: Get joypad input. */
		while(SDL_PollEvent(&event))
		{
			switch(event.type)
			{
				case SDL_QUIT:
					running = 0;
					break;
				
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym)
					{
						case SDLK_RETURN: gb.joypad_bits.start = 0; break;
						case SDLK_BACKSPACE: gb.joypad_bits.select = 0; break;
						case SDLK_z: gb.joypad_bits.a = 0; break;
						case SDLK_x: gb.joypad_bits.b = 0; break;
						case SDLK_UP: gb.joypad_bits.up = 0; break;
						case SDLK_DOWN: gb.joypad_bits.down = 0; break;
						case SDLK_LEFT: gb.joypad_bits.left = 0; break;
						case SDLK_RIGHT: gb.joypad_bits.right = 0; break;
						default: break;
					}
					break;
				case SDL_KEYUP:
					switch(event.key.keysym.sym)
					{
						case SDLK_RETURN: gb.joypad_bits.start = 1; break;
						case SDLK_BACKSPACE: gb.joypad_bits.select = 1; break;
						case SDLK_z: gb.joypad_bits.a = 1; break;
						case SDLK_x: gb.joypad_bits.b = 1; break;
						case SDLK_UP: gb.joypad_bits.up = 1; break;
						case SDLK_DOWN: gb.joypad_bits.down = 1; break;
						case SDLK_LEFT: gb.joypad_bits.left = 1; break;
						case SDLK_RIGHT: gb.joypad_bits.right = 1; break;
						default: break;
					}
					break;
			}
			if(event.type == SDL_QUIT)
				running = 0;
		}

		/* Calculate the time taken to draw frame, then later add a delay to cap
		 * at 60 fps. */
		old_ticks = SDL_GetTicks();

		gb_process_joypad(&gb);

		/* Execute CPU cycles until the screen has to be redrawn. */
		gb_run_frame(&gb);

		/* Copy frame buffer from emulator context, converting to colours
		 * defined in the palette. */
		for (unsigned int y = 0; y < height; y++)
		{
			for (unsigned int x = 0; x < width; x++)
				fb[y][x] = palette[gb.gb_fb[y][x] & 3];
		}

		/* Copy frame buffer to SDL screen. */
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

		/* Use a delay that will draw the screen at a rate of 59.73 Hz. */
		new_ticks = SDL_GetTicks();
		delay = 16 - (new_ticks - old_ticks);
		SDL_Delay(delay > 0 ? delay : 0);
	}

	SDL_Quit();

	/* Record save file. */
	write_cart_ram_file(save_file_name, &priv.cart_ram, gb_get_save_size(&gb));

	free(priv.rom);
	free(priv.cart_ram);
	free(save_file_name);

	return EXIT_SUCCESS;
}
