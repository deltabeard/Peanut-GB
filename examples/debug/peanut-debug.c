/**
 * MIT License
 * Copyright (c) 2018-2023 Mahyar Koshkouei
 *
 * A more bare-bones application to help with debugging.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SDL.h"

#define PEANUT_GB_USE_NIBBLE_FOR_PALETTE 1
#define ENABLE_LCD 1
#define ENABLE_SOUND 0

#include "../../peanut_gb.h"

const uint16_t lcd_palette[4 * 4] =
{
	0x7FFF, 0x5294, 0x294A, 0x0000,
	0x7FFF, 0x5294, 0x294A, 0x0000,
	0x7FFF, 0x5294, 0x294A, 0x0000
};

struct priv_t
{
	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
	/* Pointer to allocated memory holding save file. */
	uint8_t *cart_ram;
	uint16_t fb[LCD_HEIGHT][LCD_WIDTH];
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

/**
 * Handles an error reported by the emulator. The emulator context may be used
 * to better understand why the error given in gb_err was reported.
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
		   const uint_fast8_t line)
{
	struct priv_t *priv = gb->direct.priv;

	for(unsigned int x = 0; x < LCD_WIDTH; x++)
	{
		priv->fb[line][x] = lcd_palette[pixels[x] & (LCD_COLOUR | LCD_PALETTE_ALL)];
	}
}
#endif

int main(int argc, char **argv)
{
	struct gb_s gb;
	struct priv_t priv;
	const unsigned int height = 144;
	const unsigned int width = 160;
	unsigned int running = 1;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	SDL_Event event;
	uint32_t new_ticks, old_ticks;
	char *save_file_name;
	enum gb_init_error_e ret;
	unsigned int fast_mode = 0;
	unsigned int debug_mode = 1;

	/* Make sure a file name is given. */
	if(argc < 2 || argc > 3)
	{
		printf("Usage: %s FILE [SAVE]\n", argv[0]);
		puts("SAVE is set by default if not provided.");
		return EXIT_FAILURE;
	}

	/* Copy input ROM file to allocated memory. */
	if((priv.rom = read_rom_to_ram(argv[1])) == NULL)
	{
		printf("%d: %s\n", __LINE__, strerror(errno));
		return EXIT_FAILURE;
	}

	/* If no save file is specified, copy save file (with specific name) to
	 * allocated memory. */
	if(argc == 2)
	{
		char *str_replace;
		const char extension[] = ".sav";

		/* Allocate enough space for the ROM file name, for the "sav" extension
		 * and for the null terminator. */
		save_file_name = malloc(strlen(argv[1]) + strlen(extension) + 1);

		if(save_file_name == NULL)
		{
			printf("%d: %s\n", __LINE__, strerror(errno));
			return EXIT_FAILURE;
		}

		/* Copy the ROM file name to allocated space. */
		strcpy(save_file_name, argv[1]);

		/* If the file name does not have a dot, or the only dot is at the start
		 * of the file name, set the pointer to begin replacing the string to
		 * the end of the file name, otherwise set it to the dot. */
		if((str_replace = strrchr(save_file_name, '.')) == NULL ||
				str_replace == save_file_name)
			str_replace = save_file_name + strlen(save_file_name);

		/* Copy extension to string including terminating null byte. */
		for(unsigned int i = 0; i <= strlen(extension); i++)
			*(str_replace++) = extension[i];
	}
	else
		save_file_name = argv[2];

	/* TODO: Sanity check input GB file. */

	/* Initialise emulator context. */
	ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write,
			&gb_error, &priv);

	if(ret != GB_INIT_NO_ERROR)
	{
		printf("Unable to initialise context. Returned %d.\n", ret);
		exit(EXIT_FAILURE);
	}

	/* Load Save File. */
	read_cart_ram_file(save_file_name, &priv.cart_ram, gb_get_save_size(&gb));

#if ENABLE_LCD
	gb_init_lcd(&gb, &lcd_draw_line);
#endif
	/* Initialise frontend implementation, in this case, SDL2. */
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Could not initialise SDL: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	window = SDL_CreateWindow("DMG Emulator",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width, height,
			0);
	if(window == NULL)
	{
		printf("Could not create window: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if(renderer == NULL)
	{
		printf("Could not create renderer: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	if(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) < 0)
	{
		printf("Renderer could not draw color: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	if(SDL_RenderClear(renderer) < 0)
	{
		printf("Renderer could not clear: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	SDL_RenderPresent(renderer);

	texture = SDL_CreateTexture(renderer,
			SDL_PIXELFORMAT_RGB565,
			SDL_TEXTUREACCESS_STREAMING,
			width, height);
	if(texture == NULL)
	{
		printf("Texture could not be created: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	new_ticks = SDL_GetTicks();

	uint8_t pc_log[2] = { 0xFF, 0xFF };
	bool pc_log_count = false;

	while(running)
	{
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
						case SDLK_RETURN: gb.direct.joypad &= ~JOYPAD_START; break;
						case SDLK_BACKSPACE: gb.direct.joypad &= ~JOYPAD_SELECT; break;
						case SDLK_z: gb.direct.joypad &= ~JOYPAD_A; break;
						case SDLK_x: gb.direct.joypad &= ~JOYPAD_B; break;
						case SDLK_UP: gb.direct.joypad &= ~JOYPAD_UP; break;
						case SDLK_DOWN: gb.direct.joypad &= ~JOYPAD_DOWN; break;
						case SDLK_LEFT: gb.direct.joypad &= ~JOYPAD_LEFT; break;
						case SDLK_RIGHT: gb.direct.joypad &= ~JOYPAD_RIGHT; break;
						case SDLK_SPACE: fast_mode = !fast_mode; break;
						case SDLK_d: debug_mode = !debug_mode; break;
						default: break;
					}
					break;
				case SDL_KEYUP:
					switch(event.key.keysym.sym)
					{
						case SDLK_RETURN: gb.direct.joypad |= JOYPAD_START; break;
						case SDLK_BACKSPACE: gb.direct.joypad |= JOYPAD_SELECT; break;
						case SDLK_z: gb.direct.joypad |= JOYPAD_A; break;
						case SDLK_x: gb.direct.joypad |= JOYPAD_B; break;
						case SDLK_UP: gb.direct.joypad |= JOYPAD_UP; break;
						case SDLK_DOWN: gb.direct.joypad |= JOYPAD_DOWN; break;
						case SDLK_LEFT: gb.direct.joypad |= JOYPAD_LEFT; break;
						case SDLK_RIGHT: gb.direct.joypad |= JOYPAD_RIGHT; break;
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

		/* Execute CPU cycles until the screen has to be redrawn. */
		//gb_run_frame(&gb);
		
		gb.gb_frame = false;
		while(!gb.gb_frame)
		{
			const char *lcd_mode_str[4] = {
				"HBLANK", "VBLANK", "OAM", "TRANSFER"
			};
			__gb_step_cpu(&gb);

			if(debug_mode == 0)
				continue;

			/* Debugging */
			printf("OP:%02X%s PC:%04X A:%02X BC:%04X DE:%04X SP:%04X HL:%04X ",
					__gb_read(&gb, gb.cpu_reg.pc.reg),
					gb.gb_halt ? "(HALTED)" : "",
					gb.cpu_reg.pc.reg,
					gb.cpu_reg.a,
					gb.cpu_reg.bc.reg,
					gb.cpu_reg.de.reg,
					gb.cpu_reg.sp.reg,
					gb.cpu_reg.hl.reg);
			printf("LCD Mode: %s, LCD Power: %s ",
					lcd_mode_str[gb.hram_io[IO_STAT] & STAT_MODE],
					(gb.hram_io[IO_LCDC] >> 7) ? "ON" : "OFF");
			printf("ROM%d", gb.selected_rom_bank);
			printf("\n");

			pc_log[pc_log_count] = __gb_read(&gb, gb.cpu_reg.pc.reg);
			pc_log_count = !pc_log_count;
			if(pc_log[0] == 0x00 && pc_log[1] == 0x00)
			{
				puts("NOP Slide detected.");
				goto quit;
			}
		}

		/* Copy frame buffer to SDL screen. */
		SDL_UpdateTexture(texture, NULL, priv.fb, width * sizeof(uint16_t));
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);

		/* Use a delay that will draw the screen at a rate of 59.73 Hz. */
		new_ticks = SDL_GetTicks();

		if(fast_mode)
			continue;

		delay = 17 - (new_ticks - old_ticks);
		SDL_Delay(delay > 0 ? delay : 0);
	}

quit:
	SDL_Quit();

	/* Record save file. */
	write_cart_ram_file(save_file_name, &priv.cart_ram, gb_get_save_size(&gb));

	free(priv.rom);
	free(priv.cart_ram);
	free(save_file_name);

	return EXIT_SUCCESS;
}
