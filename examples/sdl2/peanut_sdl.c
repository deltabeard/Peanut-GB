/**
 * MIT License
 * Copyright (c) 2018 Mahyar Koshkouei
 *
 * An example of using the peanut_gb.h library. This example application uses
 * SDL2 to draw the screen and get input.
 */

#include <errno.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>

#if ENABLE_SOUND
#include "gb_apu/audio.h"
#endif

#include "../../peanut_gb.h"

#include "nativefiledialog/src/include/nfd.h"

//#define debugprintf printf
#define debugprintf(...)

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
void gb_error(struct gb_t *gb, const enum gb_error_e gb_err, const uint16_t val)
{
	struct priv_t *priv = gb->priv;

	switch(gb_err)
	{
		case GB_INVALID_OPCODE:
			/* We compensate for the post-increment in the __gb_step_cpu
			 * function. */
			fprintf(stdout, "Invalid opcode %#04x at PC: %#06x, SP: %#06x\n",
					val,
					gb->cpu_reg.pc - 1,
					gb->cpu_reg.sp);
			break;

			/* Ignoring non fatal errors. */
		case GB_INVALID_WRITE:
		case GB_INVALID_READ:
			return;

		default:
			printf("Unknown error");
			break;
	}

	fprintf(stderr, "Error. Press q to exit, or any other key to continue.");
	if(getchar() == 'q')
	{
		/* Record save file. */
		write_cart_ram_file("recovery.sav", &priv->cart_ram,
				gb_get_save_size(gb));

		free(priv->rom);
		free(priv->cart_ram);
		exit(EXIT_FAILURE);
	}

	return;
}

int main(int argc, char **argv)
{
	struct gb_t gb;
	struct priv_t priv;
	const unsigned int height = 144;
	const unsigned int width = 160;
	const double target_speed_ms = 1000.0/VERTICAL_SYNC;
	double speed_compensation = 0.0;
	unsigned int running = 1;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	SDL_Event event;
	SDL_Joystick *joystick;
	uint16_t fb[height][width];
	uint32_t new_ticks, old_ticks;
	enum gb_init_error_e ret;
	unsigned int fast_mode = 1;
	unsigned int fast_mode_timer = 1;
	/* Record save file every 60 seconds. */
	int save_timer = 60;
	/* Must be freed */
	char *rom_file_name = NULL;
	char *save_file_name = NULL;

	switch(argc)
	{
	case 1:
		{
			/* Invoke file picker */
			nfdresult_t result =
				NFD_OpenDialog("gb,gbc", NULL, &rom_file_name);

			if(result == NFD_CANCEL)
			{
				puts("User pressed cancel.");
				exit(EXIT_FAILURE);
			}
			else if(result != NFD_OKAY)
			{
				printf("Error: %s\n", NFD_GetError());
				exit(EXIT_FAILURE);
			}
		}
		break;

	case 2:
		/* Apply file name to rom_file_name
		 * Set save_file_name to NULL. */
		rom_file_name = argv[1];
		break;

	case 3:
		/* Apply file name to rom_file_name
		 * Apply save name to save_file_name */
		rom_file_name = argv[1];
		save_file_name = argv[2];
		break;

	default:
		printf("Usage: %s [ROM] [SAVE]\n", argv[0]);
		puts("A file picker is presented if ROM is not given.");
		puts("SAVE is set by default if not provided.");
		return EXIT_FAILURE;
	}

	/* Copy input ROM file to allocated memory. */
	if((priv.rom = read_rom_to_ram(rom_file_name)) == NULL)
	{
		printf("%d: %s\n", __LINE__, strerror(errno));
		return EXIT_FAILURE;
	}

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
			printf("%d: %s\n", __LINE__, strerror(errno));
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

	/* TODO: Sanity check input GB file. */

	/* Initialise emulator context. */
	ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write,
			&gb_error, &priv);

	switch(ret)
	{
	case GB_INIT_NO_ERROR:
		break;
	case GB_INIT_CARTRIDGE_UNSUPPORTED:
		puts("Unsupported cartridge.");
		exit(EXIT_FAILURE);
	case GB_INIT_INVALID_CHECKSUM:
		puts("Invalid ROM: Checksum failure.");
		exit(EXIT_FAILURE);
	default:
		printf("Unknown error: %d\n", ret);
		exit(EXIT_FAILURE);
	}

	/* Load Save File. */
	read_cart_ram_file(save_file_name, &priv.cart_ram, gb_get_save_size(&gb));

	/* Set the RTC of the game cartridge. Only used by games that support it. */
	{
		time_t rawtime;
		struct tm *timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);

		/* You could potentially force the game to allow the player to reset the
		 * time by setting the RTC to invalid values.
		 *
		 * Using memset(&gb->cart_rtc, 0xFF, sizeof(gb->cart_rtc)) for example
		 * causes Pokemon Gold/Silver to say "TIME NOT SET", allowing the player
		 * to set the time without having some dumb password.
		 *
		 * The memset has to be done directly to gb->cart_rtc because
		 * gb_set_rtc() processes the input values, which may cause games to not
		 * detect invalid values.
		 */

		/* Set RTC. Only games that specify support for RTC will use these
		 * values. */
		gb_set_rtc(&gb, timeinfo);
	}

	/* Initialise frontend implementation, in this case, SDL2. */
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO) < 0)
	{
		printf("Could not initialise SDL: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

#if ENABLE_SOUND
	SDL_AudioDeviceID dev;
	audio_init(&dev);
#endif

	/* Allow the joystick input even if game is in background. */
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

	/* If joystick is connected, attempt to use it. */
	joystick = SDL_JoystickOpen(0);
	if(joystick)
		printf("Joystick %s connected.\n", SDL_JoystickNameForIndex(0));

	window = SDL_CreateWindow("Peanut-sdl",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width, height,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS);
	if(window == NULL)
	{
		printf("Could not create window: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
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

	/* Use integer scale. */
	SDL_RenderSetLogicalSize(renderer, width, height);
	SDL_RenderSetIntegerScale(renderer, 1);

	texture = SDL_CreateTexture(renderer,
			SDL_PIXELFORMAT_RGB565,
			SDL_TEXTUREACCESS_STREAMING,
			width, height);
	if(texture == NULL)
	{
		printf("Texture could not be created: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	while(running)
	{
		int delay;
		static unsigned int rtc_timer = 0;
		static uint8_t selected_palette = 4;
#define MAX_PALETTE 6
		const uint16_t palette[MAX_PALETTE][4] =
		{										/* CGB Palette Entry (Notes) */
			{ 0xFFFF, 0x57E0, 0xFA00, 0x0000 },	/* 0x05 */
			{ 0xFFFF, 0xFFE0, 0xF800, 0x0000 },	/* 0x07 */
			{ 0xFFFF, 0xFD6C, 0x8180, 0x0000 },	/* 0x12 */
			{ 0x0000, 0x0430, 0xFEE0, 0xFFFF },	/* 0x13 */
			{ 0xFFFF, 0xA534, 0x528A, 0x0000 },	/* 0x16 (DMG, Default) */
			{ 0xFFF4, 0xFCB2, 0x94BF, 0x0000 }	/* 0x17 */
			/* Entries with different palettes for BG, OBJ0 & OBJ1 are not
			 * yet supported. */
		};

		/* Calculate the time taken to draw frame, then later add a
		 * delay to cap at 60 fps. */
		old_ticks = SDL_GetTicks();

		/* Get joypad input. */
		while(SDL_PollEvent(&event))
		{
			if(event.key.repeat)
				break;

			switch(event.type)
			{
				case SDL_QUIT:
					running = 0;
					break;

				case SDL_JOYHATMOTION:
					/* Reset axis when joypad hat position changed. */
					gb.joypad_bits.up = 1;
					gb.joypad_bits.right = 1;
					gb.joypad_bits.down = 1;
					gb.joypad_bits.left = 1;

					switch(event.jhat.value)
					{
						/* TODO: Diagonal cases. */
						case SDL_HAT_UP: gb.joypad_bits.up = 0; break;
						case SDL_HAT_RIGHT: gb.joypad_bits.right = 0; break;
						case SDL_HAT_DOWN: gb.joypad_bits.down = 0; break;
						case SDL_HAT_LEFT: gb.joypad_bits.left = 0; break;
					}
					break;

				case SDL_JOYBUTTONDOWN:
					switch(event.jbutton.button)
					{
						/* Button mappings I use for X-Box 360 controller. */
						case 0: gb.joypad_bits.a = 0; break;
						case 1: gb.joypad_bits.b = 0; break;
						case 6: gb.joypad_bits.select = 0; break;
						case 7: gb.joypad_bits.start = 0; break;
					}
					break;

				case SDL_JOYBUTTONUP:
					switch(event.jbutton.button)
					{
						case 0: gb.joypad_bits.a = 1; break;
						case 1: gb.joypad_bits.b = 1; break;
						case 6: gb.joypad_bits.select = 1; break;
						case 7: gb.joypad_bits.start = 1; break;
					}
					break;

				case SDL_KEYDOWN:
					switch(event.key.keysym.sym)
					{
						case SDLK_RETURN: gb.joypad_bits.start = 0; break;
						case SDLK_BACKSPACE: gb.joypad_bits.select = 0; break;
						case SDLK_z: gb.joypad_bits.a = 0; break;
						case SDLK_x: gb.joypad_bits.b = 0; break;
						case SDLK_UP: gb.joypad_bits.up = 0; break;
						case SDLK_RIGHT: gb.joypad_bits.right = 0; break;
						case SDLK_DOWN: gb.joypad_bits.down = 0; break;
						case SDLK_LEFT: gb.joypad_bits.left = 0; break;
						case SDLK_SPACE: fast_mode = 2; break;
						case SDLK_1: fast_mode = 1; break;
						case SDLK_2: fast_mode = 2; break;
						case SDLK_3: fast_mode = 3; break;
						case SDLK_4: fast_mode = 4; break;
						case SDLK_r: gb_reset(&gb); break;
						case SDLK_p:
							if(++selected_palette == MAX_PALETTE)
								selected_palette = 0;
							break;
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
						case SDLK_RIGHT: gb.joypad_bits.right = 1; break;
						case SDLK_DOWN: gb.joypad_bits.down = 1; break;
						case SDLK_LEFT: gb.joypad_bits.left = 1; break;
						case SDLK_SPACE: fast_mode = 1; break;
						case SDLK_F11:
						{
							static int fullscreen = 0;
							if(fullscreen)
							{
								SDL_SetWindowFullscreen(window, 0);
								fullscreen = 0;
							}
							else
							{
								SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
								fullscreen = SDL_WINDOW_FULLSCREEN;
							}
						}
						break;
					}
					break;
			}
		}

		/* Calculate the time taken to draw frame, then later add a
		 * delay to cap at 60 fps. */
		old_ticks = SDL_GetTicks();

		/* Execute CPU cycles until the screen has to be redrawn. */
		gb_run_frame(&gb);

		/* Tick the internal RTC when 1 second has passed. */
		rtc_timer += target_speed_ms/fast_mode;
		if(rtc_timer >= 1000)
		{
			rtc_timer -= 1000;
			gb_tick_rtc(&gb);
		}

		/* Skip frames during fast mode. */
		if(fast_mode_timer > 1)
		{
			fast_mode_timer--;
			/* We continue here since the rest of the logic in the
			 * loop is for drawing the screen and delaying. */
			continue;
		}

		fast_mode_timer = fast_mode;

#if ENABLE_SOUND
		/* Process audio. */
		audio_frame();
#endif

		/* Copy frame buffer from emulator context, converting to colours
		 * defined in the palette. */
		for (unsigned int y = 0; y < height; y++)
		{
			for (unsigned int x = 0; x < width; x++)
				fb[y][x] = palette[selected_palette][gb.gb_fb[y][x] & 3];
		}

		/* Copy frame buffer to SDL screen. */
		SDL_UpdateTexture(texture, NULL, &fb, width * sizeof(uint16_t));
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);

		/* Use a delay that will draw the screen at a rate of 59.7275 Hz. */
		new_ticks = SDL_GetTicks();

		/* Since we can only delay for a maximum resolution of 1ms, we
		 * can accumulate the error and compensate for the delay
		 * accuracy when the delay compensation surpasses 1ms. */
		speed_compensation += target_speed_ms - (new_ticks - old_ticks);

		/* We cast the delay compensation value to an integer, since it
		 * is the type used by SDL_Delay. This is where delay accuracy
		 * is lost. */
		delay = (int)(speed_compensation);

		/* We then subtract the actual delay value by the requested
		 * delay value. */
		speed_compensation -= delay;
#if ENABLE_SOUND
		debugprintf("delay: %d\t\tspeed_compensation: %f\t\taudio_len:%d",
				delay, speed_compensation, audio_length());
#endif

		/* Only run delay logic if required. */
		if(delay > 0)
		{
			uint32_t delay_ticks = SDL_GetTicks();
			uint32_t after_delay_ticks;

			/* Tick the internal RTC when 1 second has passed. */
			rtc_timer += delay;
			if(rtc_timer >= 1000)
			{
				rtc_timer -= 1000;
				gb_tick_rtc(&gb);

				/* If 60 seconds has passed, record save file.
				 * We do this because the external audio library
				 * used contains asserts that will abort the
				 * program without save.
				 * TODO: Remove use of assert in audio library
				 * in release build. */
				--save_timer;
				if(!save_timer)
				{
#if ENABLE_SOUND
					/* Locking the audio thread to reduce
					 * possibility of abort during save. */
					SDL_LockAudioDevice(dev);
#endif
					write_cart_ram_file(save_file_name,
							&priv.cart_ram,
							gb_get_save_size(&gb));
#if ENABLE_SOUND
					SDL_UnlockAudioDevice(dev);
#endif
					save_timer = 60;
				}
			}

			/* This will delay for at least the number of
			 * milliseconds requested, so we have to compensate for
			 * error here too. */
			SDL_Delay(delay);

			after_delay_ticks = SDL_GetTicks();
			speed_compensation += (double)delay -
				(int)(after_delay_ticks - delay_ticks);
			debugprintf("\t\tspeed_compensation: %f\n",
					speed_compensation);
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_DestroyTexture(texture);
	SDL_JoystickClose(joystick);
	SDL_Quit();
#if ENABLE_SOUND
	audio_cleanup();
#endif

	/* Record save file. */
	write_cart_ram_file(save_file_name, &priv.cart_ram, gb_get_save_size(&gb));

	free(priv.rom);
	free(priv.cart_ram);
	
	/* If the save file name was automatically generated (which required memory
	 * allocated on the help), then free it here. */
	if(argc == 2)
		free(save_file_name);

	if(argc == 1)
		free(rom_file_name);

	return EXIT_SUCCESS;
}
