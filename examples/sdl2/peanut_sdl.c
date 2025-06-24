/**
 * MIT License
 * Copyright (c) 2018-2023 Mahyar Koshkouei
 *
 * An example of using the peanut_gb.h library. This example application uses
 * SDL2 to draw the screen and get input.
 */

#include <stdint.h>
#include <stdlib.h>

#include "SDL.h"

#if defined(ENABLE_SOUND_BLARGG)
#	include "blargg_apu/audio.h"
#elif defined(ENABLE_SOUND_MINIGB)
#	include "minigb_apu/minigb_apu.h"
#endif

uint8_t audio_read(uint16_t addr);
void audio_write(uint16_t addr, uint8_t val);

#include "../../peanut_gb.h"

enum {
	LOG_CATERGORY_PEANUTSDL = SDL_LOG_CATEGORY_CUSTOM
};

struct priv_t
{
	/* Window context used to generate message boxes. */
	SDL_Window *win;

	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
	/* Pointer to allocated memory holding save file. */
	uint8_t *cart_ram;
	/* Size of the cart_ram in bytes. */
	size_t save_size;
	/* Pointer to boot ROM binary if available. */
	uint8_t *bootrom;

	/* Colour palette for each BG, OBJ0, and OBJ1. */
	uint16_t selected_palette[3][4];
	uint16_t fb[LCD_HEIGHT][LCD_WIDTH];
};

static struct minigb_apu_ctx apu;

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

uint8_t gb_bootrom_read(struct gb_s *gb, const uint_fast16_t addr)
{
	const struct priv_t * const p = gb->direct.priv;
	return p->bootrom[addr];
}

uint8_t audio_read(uint16_t addr)
{
	return minigb_apu_audio_read(&apu, addr);
}

void audio_write(uint16_t addr, uint8_t val)
{
	minigb_apu_audio_write(&apu, addr, val);
}

void audio_callback(void *ptr, uint8_t *data, int len)
{
	minigb_apu_audio_callback(&apu, (void *)data);
}

void read_cart_ram_file(const char *save_file_name, uint8_t **dest,
			const size_t len)
{
	SDL_RWops *f;

	/* If save file not required. */
	if(len == 0)
	{
		*dest = NULL;
		return;
	}

	/* Allocate enough memory to hold save file. */
	if((*dest = SDL_malloc(len)) == NULL)
	{
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"%d: %s", __LINE__, SDL_GetError());
		exit(EXIT_FAILURE);
	}

	f = SDL_RWFromFile(save_file_name, "rb");

	/* It doesn't matter if the save file doesn't exist. We initialise the
	 * save memory allocated above. The save file will be created on exit. */
	if(f == NULL)
	{
		SDL_memset(*dest, 0, len);
		return;
	}

	/* Read save file to allocated memory. */
	SDL_RWread(f, *dest, sizeof(uint8_t), len);
	SDL_RWclose(f);
}

void write_cart_ram_file(const char *save_file_name, uint8_t **dest,
			 const size_t len)
{
	SDL_RWops *f;

	if(len == 0 || *dest == NULL)
		return;

	if((f = SDL_RWFromFile(save_file_name, "wb")) == NULL)
	{
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"Unable to open save file: %s",
				SDL_GetError());
		return;
	}

	/* Record save file. */
	SDL_RWwrite(f, *dest, sizeof(uint8_t), len);
	SDL_RWclose(f);

	return;
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
		""
	};
	struct priv_t *priv = gb->direct.priv;
	char error_msg[256];
	char location[64] = "";
	uint8_t instr_byte;

	/* Record save file. */
	write_cart_ram_file("recovery.sav", &priv->cart_ram, priv->save_size);

	if(addr >= 0x4000 && addr < 0x8000)
	{
		uint32_t rom_addr;
		rom_addr = (uint32_t)addr * (uint32_t)gb->selected_rom_bank;
		SDL_snprintf(location, sizeof(location),
			" (bank %d mode %d, file offset %u)",
			gb->selected_rom_bank, gb->cart_mode_select, rom_addr);
	}

	instr_byte = __gb_read(gb, addr);

	SDL_snprintf(error_msg, sizeof(error_msg),
		"Error: %s at 0x%04X%s with instruction %02X.\n"
		"Cart RAM saved to recovery.sav\n"
		"Exiting.\n",
		gb_err_str[gb_err], addr, location, instr_byte);
	SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
			SDL_LOG_PRIORITY_CRITICAL,
			"%s", error_msg);

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", error_msg, priv->win);

	/* Free memory and then exit. */
	SDL_free(priv->cart_ram);
	SDL_free(priv->rom);
	exit(EXIT_FAILURE);
}

/**
 * Automatically assigns a colour palette to the game using a given game
 * checksum.
 * TODO: Not all checksums are programmed in yet because I'm lazy.
 */
void auto_assign_palette(struct priv_t *priv, uint8_t game_checksum)
{
	size_t palette_bytes = 3 * 4 * sizeof(uint16_t);

	switch(game_checksum)
	{
	/* Balloon Kid and Tetris Blast */
	case 0x71:
	case 0xFF:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7E60, 0x7C00, 0x0000 }, /* OBJ0 */
			{ 0x7FFF, 0x7E60, 0x7C00, 0x0000 }, /* OBJ1 */
			{ 0x7FFF, 0x7E60, 0x7C00, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Pokemon Yellow and Tetris */
	case 0x15:
	case 0xDB:
	case 0x95: /* Not officially */
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7FE0, 0x7C00, 0x0000 }, /* OBJ0 */
			{ 0x7FFF, 0x7FE0, 0x7C00, 0x0000 }, /* OBJ1 */
			{ 0x7FFF, 0x7FE0, 0x7C00, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Donkey Kong */
	case 0x19:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }, /* OBJ0 */
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }, /* OBJ1 */
			{ 0x7FFF, 0x7E60, 0x7C00, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Pokemon Blue */
	case 0x61:
	case 0x45:

	/* Pokemon Blue Star */
	case 0xD8:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }, /* OBJ0 */
			{ 0x7FFF, 0x329F, 0x001F, 0x0000 }, /* OBJ1 */
			{ 0x7FFF, 0x329F, 0x001F, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Pokemon Red */
	case 0x14:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x3FE6, 0x0200, 0x0000 }, /* OBJ0 */
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }, /* OBJ1 */
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Pokemon Red Star */
	case 0x8B:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }, /* OBJ0 */
			{ 0x7FFF, 0x329F, 0x001F, 0x0000 }, /* OBJ1 */
			{ 0x7FFF, 0x3FE6, 0x0200, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Kirby */
	case 0x27:
	case 0x49:
	case 0x5C:
	case 0xB3:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7D8A, 0x6800, 0x3000, 0x0000 }, /* OBJ0 */
			{ 0x001F, 0x7FFF, 0x7FEF, 0x021F }, /* OBJ1 */
			{ 0x527F, 0x7FE0, 0x0180, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Donkey Kong Land [1/2/III] */
	case 0x18:
	case 0x6A:
	case 0x4B:
	case 0x6B:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7F08, 0x7F40, 0x48E0, 0x2400 }, /* OBJ0 */
			{ 0x7FFF, 0x2EFF, 0x7C00, 0x001F }, /* OBJ1 */
			{ 0x7FFF, 0x463B, 0x2951, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Link's Awakening */
	case 0x70:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x03E0, 0x1A00, 0x0120 }, /* OBJ0 */
			{ 0x7FFF, 0x329F, 0x001F, 0x001F }, /* OBJ1 */
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Mega Man [1/2/3] & others I don't care about. */
	case 0x01:
	case 0x10:
	case 0x29:
	case 0x52:
	case 0x5D:
	case 0x68:
	case 0x6D:
	case 0xF6:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x329F, 0x001F, 0x0000 }, /* OBJ0 */
			{ 0x7FFF, 0x3FE6, 0x0200, 0x0000 }, /* OBJ1 */
			{ 0x7FFF, 0x7EAC, 0x40C0, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	default:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
			{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
			{ 0x7FFF, 0x5294, 0x294A, 0x0000 }
		};
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_INFO,
				"No palette found for 0x%02X.", game_checksum);
		memcpy(priv->selected_palette, palette, palette_bytes);
	}
	}
}

/**
 * Assigns a palette. This is used to allow the user to manually select a
 * different colour palette if one was not found automatically, or if the user
 * prefers a different colour palette.
 * selection is the requestion colour palette. This should be a maximum of
 * NUMBER_OF_PALETTES - 1. The default greyscale palette is selected otherwise.
 */
void manual_assign_palette(struct priv_t *priv, uint8_t selection)
{
#define NUMBER_OF_PALETTES 12
	size_t palette_bytes = 3 * 4 * sizeof(uint16_t);

	switch(selection)
	{
	/* 0x05 (Right) */
	case 0:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x2BE0, 0x7D00, 0x0000 },
			{ 0x7FFF, 0x2BE0, 0x7D00, 0x0000 },
			{ 0x7FFF, 0x2BE0, 0x7D00, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x07 (A + Down) */
	case 1:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7FE0, 0x7C00, 0x0000 },
			{ 0x7FFF, 0x7FE0, 0x7C00, 0x0000 },
			{ 0x7FFF, 0x7FE0, 0x7C00, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x12 (Up) */
	case 2:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7EAC, 0x40C0, 0x0000 },
			{ 0x7FFF, 0x7EAC, 0x40C0, 0x0000 },
			{ 0x7FFF, 0x7EAC, 0x40C0, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x13 (B + Right) */
	case 3:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x0000, 0x0210, 0x7F60, 0x7FFF },
			{ 0x0000, 0x0210, 0x7F60, 0x7FFF },
			{ 0x0000, 0x0210, 0x7F60, 0x7FFF }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x16 (B + Left, DMG Palette) */
	default:
	case 4:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
			{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
			{ 0x7FFF, 0x5294, 0x294A, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x17 (Down) */
	case 5:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FF4, 0x7E52, 0x4A5F, 0x0000 },
			{ 0x7FF4, 0x7E52, 0x4A5F, 0x0000 },
			{ 0x7FF4, 0x7E52, 0x4A5F, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x19 (B + Up) */
	case 6:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7EAC, 0x40C0, 0x0000 },
			{ 0x7FFF, 0x7EAC, 0x40C0, 0x0000 },
			{ 0x7F98, 0x6670, 0x41A5, 0x2CC1 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x1C (A + Right) */
	case 7:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 },
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 },
			{ 0x7FFF, 0x3FE6, 0x0198, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x0D (A + Left) */
	case 8:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 },
			{ 0x7FFF, 0x7EAC, 0x40C0, 0x0000 },
			{ 0x7FFF, 0x463B, 0x2951, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x10 (A + Up) */
	case 9:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x3FE6, 0x0200, 0x0000 },
			{ 0x7FFF, 0x329F, 0x001F, 0x0000 },
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x18 (Left) */
	case 10:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 },
			{ 0x7FFF, 0x3FE6, 0x0200, 0x0000 },
			{ 0x7FFF, 0x329F, 0x001F, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x1A (B + Down) */
	case 11:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x329F, 0x001F, 0x0000 },
			{ 0x7FFF, 0x3FE6, 0x0200, 0x0000 },
			{ 0x7FFF, 0x7FE0, 0x3D20, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}
	}

	return;
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
		priv->fb[line][x] = priv->selected_palette
				    [(pixels[x] & LCD_PALETTE_ALL) >> 4]
				    [pixels[x] & 3];
	}
}
#endif

/**
 * Saves the LCD screen as a 15-bit BMP file.
 */
int save_lcd_bmp(struct gb_s* gb, uint16_t fb[LCD_HEIGHT][LCD_WIDTH])
{
	/* Should be enough to record up to 828 days worth of frames. */
	static uint_fast32_t file_num = 0;
	char file_name[32];
	char title_str[16];
	SDL_RWops *f;
	int ret = -1;

	SDL_snprintf(file_name, 32, "%.16s_%010ld.bmp",
		 gb_get_rom_name(gb, title_str), file_num);

	f = SDL_RWFromFile(file_name, "wb");
	if(f == NULL)
		goto ret;

	const uint8_t bmp_hdr_rgb555[] = {
		0x42, 0x4d, 0x36, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0xa0, 0x00,
		0x00, 0x00, 0x70, 0xff, 0xff, 0xff, 0x01, 0x00, 0x10, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xb4, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
	};

	SDL_RWwrite(f, bmp_hdr_rgb555, sizeof(uint8_t), sizeof(bmp_hdr_rgb555));
	SDL_RWwrite(f, fb, sizeof(uint16_t), LCD_HEIGHT * LCD_WIDTH);
	ret = SDL_RWclose(f);

	file_num++;

ret:
	return ret;
}

int main(int argc, char **argv)
{
	struct gb_s gb;
	struct priv_t priv =
	{
		.rom = NULL,
		.cart_ram = NULL
	};
	const double target_speed_ms = 1000.0 / VERTICAL_SYNC;
	double speed_compensation = 0.0;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	SDL_Event event;
	SDL_GameController *controller = NULL;
	uint_fast32_t new_ticks, old_ticks;
	enum gb_init_error_e gb_ret;
	unsigned int fast_mode = 1;
	unsigned int fast_mode_timer = 1;
	/* Record save file every 60 seconds. */
	int save_timer = 60;
	/* Must be freed */
	char *rom_file_name = NULL;
	char *save_file_name = NULL;
	int ret = EXIT_SUCCESS;

	SDL_LogSetPriority(LOG_CATERGORY_PEANUTSDL, SDL_LOG_PRIORITY_INFO);

	/* Enable Hi-DPI to stop blurry game image. */
#ifdef SDL_HINT_WINDOWS_DPI_AWARENESS
	SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
#endif

	/* Initialise frontend implementation, in this case, SDL2. */
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) < 0)
	{
		char buf[128];
		SDL_snprintf(buf, sizeof(buf),
				"Unable to initialise SDL2: %s", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", buf, NULL);
		ret = EXIT_FAILURE;
		goto out;
	}

	window = SDL_CreateWindow("Peanut-SDL: Opening File",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			LCD_WIDTH * 2, LCD_HEIGHT * 2,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS);

	if(window == NULL)
	{
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"Could not create window: %s",
				SDL_GetError());
		ret = EXIT_FAILURE;
		goto out;
	}
	priv.win = window;

	switch(argc)
	{
	case 1:
		SDL_SetWindowTitle(window, "Drag and drop ROM");
		do
		{
			SDL_Delay(10);
			SDL_PollEvent(&event);

			switch(event.type)
			{
				case SDL_DROPFILE:
					rom_file_name = event.drop.file;
					break;

				case SDL_QUIT:
					ret = EXIT_FAILURE;
					goto out;

				default:
					break;
			}
		} while(rom_file_name == NULL);

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
#if ENABLE_FILE_GUI
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"Usage: %s [ROM] [SAVE]", argv[0]);
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"A file picker is presented if ROM is not given.");
#else
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"Usage: %s ROM [SAVE]\n", argv[0]);
#endif
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"SAVE is set by default if not provided.");
		ret = EXIT_FAILURE;
		goto out;
	}

	/* Copy input ROM file to allocated memory. */
	if((priv.rom = SDL_LoadFile(rom_file_name, NULL)) == NULL)
	{
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"%d: %s", __LINE__, SDL_GetError());
		ret = EXIT_FAILURE;
		goto out;
	}

	/* If no save file is specified, copy save file (with specific name) to
	 * allocated memory. */
	if(save_file_name == NULL)
	{
		char *str_replace;
		const char extension[] = ".sav";

		/* Allocate enough space for the ROM file name, for the "sav"
		 * extension and for the null terminator. */
		save_file_name = SDL_malloc(
				SDL_strlen(rom_file_name) + SDL_strlen(extension) + 1);

		if(save_file_name == NULL)
		{
			SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
					SDL_LOG_PRIORITY_CRITICAL,
					"%d: %s", __LINE__, SDL_GetError());
			ret = EXIT_FAILURE;
			goto out;
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
	gb_ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write,
			 &gb_error, &priv);

	switch(gb_ret)
	{
	case GB_INIT_NO_ERROR:
		break;

	case GB_INIT_CARTRIDGE_UNSUPPORTED:
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"Unsupported cartridge.");
		ret = EXIT_FAILURE;
		goto out;

	case GB_INIT_INVALID_CHECKSUM:
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"Invalid ROM: Checksum failure.");
		ret = EXIT_FAILURE;
		goto out;

	default:
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"Unknown error: %d", gb_ret);
		ret = EXIT_FAILURE;
		goto out;
	}

	/* Copy dmg_boot.bin boot ROM file to allocated memory. */
	if((priv.bootrom = SDL_LoadFile("dmg_boot.bin", NULL)) == NULL)
	{
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_INFO,
				"No dmg_boot.bin file found; disabling boot ROM");
	}
	else
	{
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_INFO,
				"boot ROM enabled");
		gb_set_bootrom(&gb, gb_bootrom_read);
		gb_reset(&gb);
	}

	/* Load Save File. */
	if(gb_get_save_size_s(&gb, &priv.save_size) < 0)
	{
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"Unable to get save size: %s",
				SDL_GetError());
		ret = EXIT_FAILURE;
		goto out;
	}

	/* Only attempt to load a save file if the ROM actually supports saves.*/
	if(priv.save_size > 0)
		read_cart_ram_file(save_file_name, &priv.cart_ram, priv.save_size);

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

#if ENABLE_SOUND
	SDL_AudioDeviceID dev;
#endif

#if ENABLE_SOUND == 0
	// Sound is disabled, so do nothing.
#elif defined(ENABLE_SOUND_BLARGG)
	audio_init(&dev);
#elif defined(ENABLE_SOUND_MINIGB)
	{
		SDL_AudioSpec want, have;

		want.freq = AUDIO_SAMPLE_RATE;
		want.format   = AUDIO_S16,
		want.channels = 2;
		want.samples = AUDIO_SAMPLES;
		want.callback = audio_callback;
		want.userdata = NULL;

		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_INFO,
				"Audio driver: %s",
				SDL_GetAudioDeviceName(0, 0));

		if((dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0)) == 0)
		{
			SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
					SDL_LOG_PRIORITY_CRITICAL,
					"SDL could not open audio device: %s",
					SDL_GetError());
			exit(EXIT_FAILURE);
		}

		minigb_apu_audio_init(&apu);
		SDL_PauseAudioDevice(dev, 0);
	}
#endif

#if ENABLE_LCD
	gb_init_lcd(&gb, &lcd_draw_line);
#endif

	/* Allow the joystick input even if game is in background. */
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

	if(SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt") < 0)
	{
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_INFO,
				"Unable to assign joystick mappings: %s\n",
				SDL_GetError());
	}

	/* Open the first available controller. */
	for(int i = 0; i < SDL_NumJoysticks(); i++)
	{
		if(!SDL_IsGameController(i))
			continue;

		controller = SDL_GameControllerOpen(i);

		if(controller)
		{
			SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
					SDL_LOG_PRIORITY_INFO,
					"Game Controller %s connected.",
					SDL_GameControllerName(controller));
			break;
		}
		else
		{
			SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
					SDL_LOG_PRIORITY_INFO,
					"Could not open game controller %i: %s\n",
					i, SDL_GetError());
		}
	}

	{
		/* 12 for "Peanut-SDL: " and a maximum of 16 for the title. */
		char title_str[28] = "Peanut-SDL: ";
		gb_get_rom_name(&gb, title_str + 12);
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_INFO,
				"%s",
				title_str);
		SDL_SetWindowTitle(window, title_str);
	}

	SDL_SetWindowMinimumSize(window, LCD_WIDTH, LCD_HEIGHT);

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	if(renderer == NULL)
	{
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"Could not create renderer: %s",
				SDL_GetError());
		ret = EXIT_FAILURE;
		goto out;
	}

	if(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) < 0)
	{
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"Renderer could not draw color: %s",
				SDL_GetError());
		ret = EXIT_FAILURE;
		goto out;
	}

	if(SDL_RenderClear(renderer) < 0)
	{
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"Renderer could not clear: %s",
				SDL_GetError());
		ret = EXIT_FAILURE;
		goto out;
	}

	SDL_RenderPresent(renderer);

	/* Use integer scale. */
	SDL_RenderSetLogicalSize(renderer, LCD_WIDTH, LCD_HEIGHT);
	SDL_RenderSetIntegerScale(renderer, 1);

	texture = SDL_CreateTexture(renderer,
				    SDL_PIXELFORMAT_RGB555,
				    SDL_TEXTUREACCESS_STREAMING,
				    LCD_WIDTH, LCD_HEIGHT);

	if(texture == NULL)
	{
		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
				SDL_LOG_PRIORITY_CRITICAL,
				"Texture could not be created: %s",
				SDL_GetError());
		ret = EXIT_FAILURE;
		goto out;
	}

	auto_assign_palette(&priv, gb_colour_hash(&gb));

	while(SDL_QuitRequested() == SDL_FALSE)
	{
		int delay;
		static double rtc_timer = 0;
		static unsigned int selected_palette = 3;
		static unsigned int dump_bmp = 0;

		/* Calculate the time taken to draw frame, then later add a
		 * delay to cap at 60 fps. */
		old_ticks = SDL_GetTicks();

		/* Get joypad input. */
		while(SDL_PollEvent(&event))
		{
			static int fullscreen = 0;

			switch(event.type)
			{
			case SDL_QUIT:
				goto quit;

			case SDL_CONTROLLERBUTTONDOWN:
				switch(event.cbutton.button)
				{
				case SDL_CONTROLLER_BUTTON_A:
					gb.direct.joypad &= ~JOYPAD_A;
					break;

				case SDL_CONTROLLER_BUTTON_B:
					gb.direct.joypad &= ~JOYPAD_B;
					break;

				case SDL_CONTROLLER_BUTTON_BACK:
					gb.direct.joypad &= ~JOYPAD_SELECT;
					break;

				case SDL_CONTROLLER_BUTTON_START:
					gb.direct.joypad &= ~JOYPAD_START;
					break;

				case SDL_CONTROLLER_BUTTON_DPAD_UP:
					gb.direct.joypad &= ~JOYPAD_UP;
					break;

				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
					gb.direct.joypad &= ~JOYPAD_RIGHT;
					break;

				case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
					gb.direct.joypad &= ~JOYPAD_DOWN;
					break;

				case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
					gb.direct.joypad &= ~JOYPAD_LEFT;
					break;
				}

				break;

			case SDL_CONTROLLERBUTTONUP:
				switch(event.cbutton.button)
				{
				case SDL_CONTROLLER_BUTTON_A:
					gb.direct.joypad |= JOYPAD_A;
					break;

				case SDL_CONTROLLER_BUTTON_B:
					gb.direct.joypad |= JOYPAD_B;
					break;

				case SDL_CONTROLLER_BUTTON_BACK:
					gb.direct.joypad |= JOYPAD_SELECT;
					break;

				case SDL_CONTROLLER_BUTTON_START:
					gb.direct.joypad |= JOYPAD_START;
					break;

				case SDL_CONTROLLER_BUTTON_DPAD_UP:
					gb.direct.joypad |= JOYPAD_UP;
					break;

				case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
					gb.direct.joypad |= JOYPAD_RIGHT;
					break;

				case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
					gb.direct.joypad |= JOYPAD_DOWN;
					break;

				case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
					gb.direct.joypad |= JOYPAD_LEFT;
					break;
				}

				break;

			case SDL_KEYDOWN:
				switch(event.key.keysym.sym)
				{
				case SDLK_RETURN:
					gb.direct.joypad &= ~JOYPAD_START;
					break;

				case SDLK_BACKSPACE:
					gb.direct.joypad &= ~JOYPAD_SELECT;
					break;

				case SDLK_z:
					gb.direct.joypad &= ~JOYPAD_A;
					break;

				case SDLK_x:
					gb.direct.joypad &= ~JOYPAD_B;
					break;

				case SDLK_a:
					gb.direct.joypad ^= JOYPAD_A;
					break;

				case SDLK_s:
					gb.direct.joypad ^= JOYPAD_B;
					break;

				case SDLK_UP:
					gb.direct.joypad &= ~JOYPAD_UP;
					break;

				case SDLK_RIGHT:
					gb.direct.joypad &= ~JOYPAD_RIGHT;
					break;

				case SDLK_DOWN:
					gb.direct.joypad &= ~JOYPAD_DOWN;
					break;

				case SDLK_LEFT:
					gb.direct.joypad &= ~JOYPAD_LEFT;
					break;

				case SDLK_SPACE:
					fast_mode = 2;
					break;

				case SDLK_1:
					fast_mode = 1;
					break;

				case SDLK_2:
					fast_mode = 2;
					break;

				case SDLK_3:
					fast_mode = 3;
					break;

				case SDLK_4:
					fast_mode = 4;
					break;

				case SDLK_r:
					gb_reset(&gb);
					break;
#if ENABLE_LCD

				case SDLK_i:
					gb.direct.interlace = !gb.direct.interlace;
					break;

				case SDLK_o:
					gb.direct.frame_skip = !gb.direct.frame_skip;
					break;

				case SDLK_b:
					dump_bmp = ~dump_bmp;

					if(dump_bmp)
						SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
								SDL_LOG_PRIORITY_INFO,
								"Dumping frames");
					else
						SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
								SDL_LOG_PRIORITY_INFO,
								"Stopped dumping frames");

					break;
#endif

				case SDLK_p:
					if(event.key.keysym.mod == KMOD_LSHIFT)
					{
						auto_assign_palette(&priv, gb_colour_hash(&gb));
						break;
					}

					if(++selected_palette == NUMBER_OF_PALETTES)
						selected_palette = 0;

					manual_assign_palette(&priv, selected_palette);
					break;
				}

				break;

			case SDL_KEYUP:
				switch(event.key.keysym.sym)
				{
				case SDLK_RETURN:
					gb.direct.joypad |= JOYPAD_START;
					break;

				case SDLK_BACKSPACE:
					gb.direct.joypad |= JOYPAD_SELECT;
					break;

				case SDLK_z:
					gb.direct.joypad |= JOYPAD_A;
					break;

				case SDLK_x:
					gb.direct.joypad |= JOYPAD_B;
					break;

				case SDLK_a:
					gb.direct.joypad |= JOYPAD_A;
					break;

				case SDLK_s:
					gb.direct.joypad |= JOYPAD_B;
					break;

				case SDLK_UP:
					gb.direct.joypad |= JOYPAD_UP;
					break;

				case SDLK_RIGHT:
					gb.direct.joypad |= JOYPAD_RIGHT;
					break;

				case SDLK_DOWN:
					gb.direct.joypad |= JOYPAD_DOWN;
					break;

				case SDLK_LEFT:
					gb.direct.joypad |= JOYPAD_LEFT;
					break;

				case SDLK_SPACE:
					fast_mode = 1;
					break;

				case SDLK_f:
					if(fullscreen)
					{
						SDL_SetWindowFullscreen(window, 0);
						fullscreen = 0;
						SDL_ShowCursor(SDL_ENABLE);
					}
					else
					{
						SDL_SetWindowFullscreen(window,		SDL_WINDOW_FULLSCREEN_DESKTOP);
						fullscreen = SDL_WINDOW_FULLSCREEN_DESKTOP;
						SDL_ShowCursor(SDL_DISABLE);
					}
					break;

				case SDLK_F11:
				{
					if(fullscreen)
					{
						SDL_SetWindowFullscreen(window, 0);
						fullscreen = 0;
						SDL_ShowCursor(SDL_ENABLE);
					}
					else
					{
						SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
						fullscreen = SDL_WINDOW_FULLSCREEN;
						SDL_ShowCursor(SDL_DISABLE);
					}
				}
				break;
				}

				break;
			}
		}

		/* Execute CPU cycles until the screen has to be redrawn. */
		gb_run_frame(&gb);

		/* Tick the internal RTC when 1 second has passed. */
		rtc_timer += target_speed_ms / (double) fast_mode;

		if(rtc_timer >= 1000.0)
		{
			rtc_timer -= 1000.0;
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

#if ENABLE_SOUND_BLARGG
		/* Process audio. */
		audio_frame();
#endif

#if ENABLE_LCD
		/* Copy frame buffer to SDL screen. */
		SDL_UpdateTexture(texture, NULL, &priv.fb, LCD_WIDTH * sizeof(uint16_t));
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);

		if(dump_bmp)
		{
			if(save_lcd_bmp(&gb, priv.fb) != 0)
			{
				SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
					       SDL_LOG_PRIORITY_ERROR,
					       "Failure dumping frame: %s",
					       SDL_GetError());
				dump_bmp = 0;
				SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
					       SDL_LOG_PRIORITY_INFO,
					       "Stopped dumping frames");
			}
		}

#endif

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

		/* Only run delay logic if required. */
		if(delay > 0)
		{
			uint_fast32_t delay_ticks = SDL_GetTicks();
			uint_fast32_t after_delay_ticks;

			/* Tick the internal RTC when 1 second has passed. */
			rtc_timer += delay;

			if(rtc_timer >= 1000)
			{
				rtc_timer -= 1000;
				gb_tick_rtc(&gb);

				/* If 60 seconds has passed, record save file.
				 * We do this because the blarrg audio library
				 * used contains asserts that will abort the
				 * program without save.
				 * TODO: Remove all workarounds due to faulty
				 * external libraries. */
				--save_timer;

				if(!save_timer)
				{
#if ENABLE_SOUND_BLARGG
					/* Locking the audio thread to reduce
					 * possibility of abort during save. */
					SDL_LockAudioDevice(dev);
#endif
					write_cart_ram_file(save_file_name,
						&priv.cart_ram,
						priv.save_size);
#if ENABLE_SOUND_BLARGG
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
		}
	}

quit:
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_DestroyTexture(texture);
	SDL_GameControllerClose(controller);
	SDL_Quit();
#ifdef ENABLE_SOUND_BLARGG
	audio_cleanup();
#endif

	/* Record save file. */
	write_cart_ram_file(save_file_name, &priv.cart_ram, priv.save_size);

out:
	SDL_free(priv.rom);
	SDL_free(priv.cart_ram);

	/* If the save file name was automatically generated (which required memory
	 * allocated on the help), then free it here. */
	if(argc == 2)
		SDL_free(save_file_name);

	if(argc == 1)
		SDL_free(rom_file_name);

	return ret;
}
