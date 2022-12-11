#include <inttypes.h>
#include <stdio.h>

#include <SDL.h>

#define ENABLE_LCD 1
#include "../../../peanut_gb.h"

#include "nuklear_proj.h"
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "nuklear_sdl_renderer.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

#define NUMBER_OF_SPRITES_IN_ROW 16
#define NUMBER_OF_SPRITES_IN_COLUMN 8
#define SPRITE_BLOCKS 3
#define NUMBER_OF_SPRITES (NUMBER_OF_SPRITES_IN_ROW * NUMBER_OF_SPRITES_IN_COLUMN * SPRITE_BLOCKS)
#define SPRITE_WIDTH 8
#define SPRITE_HEIGHT 8
#define VRAM_WIDTH (NUMBER_OF_SPRITES_IN_ROW * SPRITE_WIDTH)
#define VRAM_HEIGHT (NUMBER_OF_SPRITES_IN_COLUMN * SPRITE_BLOCKS * SPRITE_WIDTH)

/* ===============================================================
 *
 *                          EXAMPLE
 *
 * ===============================================================*/
/* This are some code examples to provide a small overview of what can be
 * done with this library. To try out an example uncomment the defines */
/*#define INCLUDE_ALL */
/*#define INCLUDE_STYLE */
/*#define INCLUDE_CALCULATOR */
/*#define INCLUDE_OVERVIEW */
/*#define INCLUDE_NODE_EDITOR */

#ifdef INCLUDE_ALL
#define INCLUDE_STYLE
#define INCLUDE_CALCULATOR
#define INCLUDE_CANVAS
#define INCLUDE_OVERVIEW
#define INCLUDE_NODE_EDITOR
#endif
#define INCLUDE_OVERVIEW

#ifdef INCLUDE_STYLE
#include "../../demo/common/style.c"
#endif
#ifdef INCLUDE_CALCULATOR
#include "../../demo/common/calculator.c"
#endif
#ifdef INCLUDE_CANVAS
#include "../../demo/common/canvas.c"
#endif
#ifdef INCLUDE_OVERVIEW
int overview(struct nk_context *ctx);
#endif
#ifdef INCLUDE_NODE_EDITOR
#include "../../demo/common/node_editor.c"
#endif

enum {
	PGBDBG_LOG_APPLICATION = SDL_LOG_CATEGORY_CUSTOM,
};

#define SDL_arraysize(array) (sizeof(array)/sizeof(array[0]))

static SDL_Renderer *renderer;
typedef struct {
	SDL_Texture *gb_lcd_tex;
	SDL_Texture *gb_vram_tex;
	void *pixels;
	int pitch;
	uint8_t *rom;
	uint8_t *ram;
} gb_priv_s;

static uint8_t gb_rom_read(struct gb_s *ctx, const uint_fast32_t addr)
{
	gb_priv_s *gb_priv;
	uint8_t *rom;

	gb_priv = ctx->direct.priv;
	rom = gb_priv->rom;
	return rom[addr];
}

static uint8_t gb_cart_ram_read(struct gb_s *ctx, const uint_fast32_t addr)
{
	gb_priv_s *gb_priv;
	uint8_t *ram;

	gb_priv = ctx->direct.priv;
	ram = gb_priv->ram;
	return ram[addr];
}

static void gb_cart_ram_write(struct gb_s *ctx, const uint_fast32_t addr,
	const uint8_t val)
{
	gb_priv_s *gb_priv;
	uint8_t *ram;

	gb_priv = ctx->direct.priv;
	ram = gb_priv->ram;
	ram[addr] = val;
}

static void gb_error(struct gb_s *ctx, const enum gb_error_e err,
	const uint16_t val)
{
	const char *err_str[] = {
		"Unknown", "Invalid opcode", "Invalid read", "Invalid write",
		"Halted forever"
	};
	SDL_LogError(PGBDBG_LOG_APPLICATION,
		"Error: %s", err_str[err]);
	SDL_assert_release(0);
}

static void render_vram_tex(SDL_Texture *tex, const struct gb_s *const gb)
{
	int ret;
	int pitch;
	const uint32_t colour_lut[4] = {
		0xFFFFFFFF, 0x7F7F7FFF, 0x2F2F2FFF, 0xFF000000
	};
	uint32_t *pixels;
	SDL_Surface *s, *tile;

	ret = SDL_LockTexture(tex, NULL, (void **)&pixels, &pitch);
	SDL_assert_always(ret == 0);

	s = SDL_CreateRGBSurfaceWithFormatFrom(pixels,
		VRAM_WIDTH, VRAM_HEIGHT, 32, pitch, SDL_PIXELFORMAT_RGBA32);
	tile = SDL_CreateRGBSurfaceWithFormat(0, SPRITE_WIDTH, SPRITE_HEIGHT,
		32, SDL_PIXELFORMAT_RGBA32);

	for(unsigned sprite = 0; sprite < NUMBER_OF_SPRITES; sprite++)
	{
		int sprite_x = (sprite % NUMBER_OF_SPRITES_IN_ROW) * SPRITE_WIDTH;
		int sprite_y = (sprite / NUMBER_OF_SPRITES_IN_ROW) * SPRITE_HEIGHT;
		unsigned sprite_addr = sprite << 4;
		uint32_t *pixels = tile->pixels;
		SDL_Rect dstrect = {
			.h = 8, .w = 8,
			.x = sprite_x, .y = sprite_y
		};

		for(; sprite_addr < (sprite + 1) << 4; sprite_addr += 2)
		{
			for(int px = 7; px >= 0; px--)
			{
				uint8_t t1 = gb->vram[sprite_addr] >> px;
				uint8_t t2 = gb->vram[sprite_addr + 1] >> px;
				uint8_t c = (t1 & 0x1) | ((t2 & 0x1) << 1);

				*pixels = colour_lut[c];
				pixels++;
			}
		}

		SDL_BlitSurface(tile, NULL, s, &dstrect);
	}

	SDL_FreeSurface(s);
	SDL_FreeSurface(tile);
	SDL_UnlockTexture(tex);
	return;
}

static void lcd_draw_line(struct gb_s *gb, const uint8_t *pixels,
	const uint_fast8_t line)
{
	const uint32_t colour_lut[4] = {
		0xFFFFFFFF, 0x7F7F7FFF, 0x2F2F2FFF, 0x00000000
	};
	gb_priv_s *priv = gb->direct.priv;
	uint32_t *tex = priv->pixels;

	for(unsigned int x = 0; x < LCD_WIDTH; x++)
	{
		tex[(line * LCD_WIDTH) + x] = colour_lut[pixels[x] & 3];
	}

	return;
}

typedef enum {
	GB_STATE_PLAYING,
	GB_STATE_PAUSED,
	GB_STATE_STEP
} gb_state_e;

void print_window_pos(struct nk_context *ctx)
{
#if 0
	struct nk_rect r;
	if(!nk_window_has_focus(ctx))
		return;

	r = nk_window_get_bounds(ctx);
	SDL_LogInfo(PGBDBG_LOG_APPLICATION,
		"Bounds x: %f, y: %f, w: %f, h: %f",
		r.x, r.y, r.w, r.h);
#endif
}

static void render_peanut_gb(struct nk_context *ctx, struct gb_s *gb)
{
	const char *const win_str_lut[] = {
		"LCD: Playing", "LCD: Paused", "LCD: Frame Step"
	};
	gb_priv_s *gb_priv = gb->direct.priv;
	static int frame_step = 0;
	static gb_state_e gb_state = GB_STATE_PAUSED;

	/* Game Boy Control */
	if(nk_begin(ctx, "Control", nk_rect(15, 210, 20 + LCD_WIDTH, 85),
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
		NK_WINDOW_SCALABLE | NK_WINDOW_TITLE |
		NK_WINDOW_MINIMIZABLE))
	{
		nk_layout_row_dynamic(ctx, 30, 2);
		if(nk_button_label(ctx, "Frame Step"))
		{
			frame_step++;
			gb_state = GB_STATE_STEP;
		}

		if(gb_state == GB_STATE_PLAYING)
		{
			if(nk_button_label(ctx, "Pause"))
				gb_state = GB_STATE_PAUSED;
		}
		else if(nk_button_label(ctx, "Run"))
		{
			gb_state = GB_STATE_PLAYING;
		}
		print_window_pos(ctx);
	}
	nk_end(ctx);

	if(gb_state == GB_STATE_PLAYING ||
		(gb_state == GB_STATE_STEP && frame_step != 0))
	{
		int ret;
		ret = SDL_LockTexture(gb_priv->gb_lcd_tex, NULL,
			&gb_priv->pixels, &gb_priv->pitch);
		SDL_assert_always(ret == 0);

		gb_run_frame(gb);

		render_vram_tex(gb_priv->gb_vram_tex, gb);
		SDL_UnlockTexture(gb_priv->gb_lcd_tex);

		frame_step = 0;
	}

	/* LCD */
	if(nk_begin_titled(ctx, "LCD", win_str_lut[gb_state],
		nk_rect(15, 15, 20 + LCD_WIDTH, 46 + LCD_HEIGHT),
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
		NK_WINDOW_SCALABLE | NK_WINDOW_TITLE |
		NK_WINDOW_NO_SCROLLBAR))
	{
		struct nk_image nk_gb_lcd;
		struct nk_command_buffer *canvas;
		struct nk_rect total_space;
		const struct nk_color grid_color = { 0xFF, 0xFF, 0xFF, 0xFF };

		/* Draw Game Boy LCD to screen. */
		canvas = nk_window_get_canvas(ctx);
		total_space = nk_window_get_content_region(ctx);
		/* Use integer scaling. */
		do
		{
			unsigned scale_w, scale_h;
			scale_w = ((unsigned)total_space.w / LCD_WIDTH);
			scale_h = ((unsigned)total_space.h / LCD_HEIGHT);

			/* If the scale is less than 1, then stretch to fill
			 * canvas. */
			if(scale_w == 0 || scale_h == 0)
			{
				if(total_space.w < total_space.h)
					total_space.h = total_space.w;
				else if(total_space.h < total_space.w)
					total_space.w = total_space.h;

				break;
			}

			scale_w = scale_w > scale_h ? scale_h : scale_w;
			scale_h = scale_h > scale_w ? scale_w : scale_h;
			total_space.w = LCD_WIDTH * scale_w;
			total_space.h = LCD_HEIGHT * scale_h;
		} while(0);

		nk_gb_lcd = nk_image_ptr(gb_priv->gb_lcd_tex);
		nk_draw_image(canvas, total_space, &nk_gb_lcd, grid_color);
		print_window_pos(ctx);
	}
	nk_end(ctx);

	/* Dissassembler */
	if(nk_begin(ctx, "Assembly",
		nk_rect(480, 210, 220, 430),
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
		NK_WINDOW_SCALABLE | NK_WINDOW_TITLE |
		NK_WINDOW_MINIMIZABLE))
	{
		static nk_bool selected[2 * 1024] = { 0 };
		nk_layout_row_dynamic(ctx, 18, 1);
		for(unsigned i = 0; i < SDL_arraysize(selected); i++)
		{
			nk_selectable_label(ctx,
				selected[i] ? "Selected" : "Unselected",
				NK_TEXT_CENTERED, &selected[i]);
		}
		print_window_pos(ctx);
	}
	nk_end(ctx);

	/* Game Boy Registers */
	if(nk_begin(ctx, "Registers", nk_rect(200, 15, 400, 46 + LCD_HEIGHT),
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
		NK_WINDOW_SCALABLE | NK_WINDOW_TITLE |
		NK_WINDOW_MINIMIZABLE))
	{
		static const char reg_labels[6][3] = {
			"A", "BC", "DE", "HL", "SP", "PC"
		};
		static char reg_str[6][5];
		int reg_str_len[6];

		reg_str_len[0] = SDL_snprintf(reg_str[0], 5, "%02X",
			gb->cpu_reg.a);
		reg_str_len[1] = SDL_snprintf(reg_str[1], 5, "%04X",
			gb->cpu_reg.bc.reg);
		reg_str_len[2] = SDL_snprintf(reg_str[2], 5, "%04X",
			gb->cpu_reg.de.reg);
		reg_str_len[3] = SDL_snprintf(reg_str[3], 5, "%04X",
			gb->cpu_reg.hl.reg);
		reg_str_len[4] = SDL_snprintf(reg_str[4], 5, "%04X",
			gb->cpu_reg.sp.reg);
		reg_str_len[5] = SDL_snprintf(reg_str[5], 5, "%04X",
			gb->cpu_reg.pc.reg);

		nk_layout_row_dynamic(ctx, 60, 7);
		for(unsigned i = 0; i < SDL_arraysize(reg_labels); i++)
		{
			if(nk_group_begin(ctx, reg_labels[i],
				NK_WINDOW_NO_SCROLLBAR |
				NK_WINDOW_BORDER))
			{
				nk_layout_row_dynamic(ctx, 25, 1);
				nk_text(ctx, reg_labels[i], 2,
					NK_TEXT_CENTERED);
				nk_text(ctx, reg_str[i], reg_str_len[i],
					NK_TEXT_CENTERED);
				nk_group_end(ctx);
			}
		}

		if(nk_group_begin(ctx, "Flags",
			NK_WINDOW_NO_SCROLLBAR |
			NK_WINDOW_BORDER))
		{
			static char flags[5] = "----";
			int flags_len;
			nk_layout_row_dynamic(ctx, 25, 1);
			nk_label(ctx, "Flags", NK_TEXT_CENTERED);

			flags_len = SDL_snprintf(flags, sizeof(flags),
				"%c%c%c%c",
				gb->cpu_reg.f_bits.c ? 'C' : '-',
				gb->cpu_reg.f_bits.h ? 'H' : '-',
				gb->cpu_reg.f_bits.n ? 'N' : '-',
				gb->cpu_reg.f_bits.z ? 'Z' : '-');
			nk_text(ctx, flags, flags_len, NK_TEXT_CENTERED);
			nk_group_end(ctx);
		}

		for(unsigned i = 0; i < 3; i++)
		{
			const char timer_str[3][5] = {
				"TIMA", "TMA", "DIV"
			};
			const uint8_t *timer_reg[3] = {
				&gb->hram_io[IO_TIMA],
				&gb->hram_io[IO_TMA],
				&gb->hram_io[IO_DIV]
			};
			static char timer_reg_str[3][3];

			if(nk_group_begin(ctx, timer_str[i],
				NK_WINDOW_NO_SCROLLBAR |
				NK_WINDOW_BORDER))
			{
				int timer_reg_str_len;
				nk_layout_row_dynamic(ctx, 25, 1);
				nk_label(ctx, timer_str[i], NK_TEXT_CENTERED);
				timer_reg_str_len = SDL_snprintf(
					timer_reg_str[i], 3,
					"%02X", *timer_reg[i]);
				nk_text(ctx, timer_reg_str[i],
					timer_reg_str_len,
					NK_TEXT_CENTERED);
				nk_group_end(ctx);
			}
		}

#if 0
		for(unsigned i = 0; i < 4; i++)
		{
			const char count_str[4][7] = {
				"LCD", "DIV", "TIMA", "SERIAL"
			};
			const uint_fast16_t *count_ptrs[4] = {
				&gb->counter.lcd_count,
				&gb->counter.div_count,
				&gb->counter.tima_count,
				&gb->counter.serial_count
			};
			static char timer_reg_str[4][16];

			if(nk_group_begin(ctx, count_str[i],
				NK_WINDOW_NO_SCROLLBAR |
				NK_WINDOW_BORDER))
			{
				int timer_reg_str_len;
				nk_layout_row_dynamic(ctx, 25, 1);
				nk_label(ctx, count_str[i], NK_TEXT_CENTERED);
				timer_reg_str_len = SDL_snprintf(
					timer_reg_str[i], 16,
					"%" PRIuFAST16, *count_ptrs[i]);
				nk_text(ctx, timer_reg_str[i],
					timer_reg_str_len,
					NK_TEXT_CENTERED);
				nk_group_end(ctx);
			}
		}
#endif
		for(unsigned i = 0; i < 2; i++)
		{
			const char count_str[2][7] = {
				"IF", "IE"
			};
			const uint8_t *count_ptrs[2] = {
				&gb->hram_io[IO_IF],
				&gb->hram_io[IO_IE]
			};
			static char timer_reg_str[2][3];

			if(nk_group_begin(ctx, count_str[i],
				NK_WINDOW_NO_SCROLLBAR |
				NK_WINDOW_BORDER))
			{
				int reg_str_len;
				nk_layout_row_dynamic(ctx, 25, 1);
				nk_label(ctx, count_str[i], NK_TEXT_CENTERED);
				reg_str_len = SDL_snprintf(
					timer_reg_str[i], 3,
					"%" PRIu8, *count_ptrs[i]);
				nk_text(ctx, timer_reg_str[i],
					reg_str_len,
					NK_TEXT_CENTERED);
				nk_group_end(ctx);
			}
		}
		print_window_pos(ctx);
	}
	nk_end(ctx);

	/* VRAM */
	if(nk_begin(ctx, "VRAM Viewer",
		nk_rect(200, 210, 270, 430),
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
		NK_WINDOW_SCALABLE | NK_WINDOW_TITLE |
		NK_WINDOW_NO_SCROLLBAR))
	{
		struct nk_image nk_gb_vram;
		struct nk_command_buffer *canvas;
		struct nk_rect total_space;
		const struct nk_color grid_color = { 0xFF, 0xFF, 0xFF, 0xFF };

		/* Draw VRAM to screen. */
		canvas = nk_window_get_canvas(ctx);
		total_space = nk_window_get_content_region(ctx);

		/* Use integer scaling. */
		do
		{
			unsigned scale_w, scale_h;
			scale_w = ((unsigned)total_space.w / VRAM_WIDTH);
			scale_h = ((unsigned)total_space.h / VRAM_HEIGHT);

			/* If the scale is less than 1, then stretch to fill
			 * canvas. */
			if(scale_w == 0 || scale_h == 0)
			{
				if(total_space.w < total_space.h)
					total_space.h = total_space.w;
				else if(total_space.h < total_space.w)
					total_space.w = total_space.h;

				break;
			}

			scale_w = scale_w > scale_h ? scale_h : scale_w;
			scale_h = scale_h > scale_w ? scale_w : scale_h;
			total_space.w = VRAM_WIDTH * scale_w;
			total_space.h = VRAM_HEIGHT * scale_h;
		} while(0);

		nk_gb_vram = nk_image_ptr(gb_priv->gb_vram_tex);
		nk_draw_image(canvas, total_space, &nk_gb_vram, grid_color);
		print_window_pos(ctx);
	}
	nk_end(ctx);
}

/**
 * Returns a pointer to the allocated space containing the ROM. Must be freed.
 */
static uint8_t *read_rom_to_ram(const char *file_name)
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
	}

out:
	fclose(rom_file);
	return rom;
}

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# include <Windows.h>

# if (_WIN32_WINNT >= 0x0603)
#  include <shellscalingapi.h>
# endif
#endif

static inline void set_dpi_awareness(void)
{
# if (_WIN32_WINNT >= 0x0605)
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
# elif (_WIN32_WINNT >= 0x0603)
	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
# elif (_WIN32_WINNT >= 0x0600)
	SetProcessDPIAware();
# elif defined(__MINGW64__)
	SetProcessDPIAware();
# endif
	return;
}

/* ===============================================================
 *
 *                          DEMO
 *
 * ===============================================================*/
int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	/* Platform */
	SDL_Window *win;
	int running = 1;
	float font_scale = 1;

	/* GUI */
	struct nk_context *ctx;
	struct nk_colorf bg;

	/* GB */
	static struct gb_s gb;
	gb_priv_s gb_priv;

	/* Print application log. */
	SDL_LogSetPriority(PGBDBG_LOG_APPLICATION, SDL_LOG_PRIORITY_INFO);

	/* Make sure a file name is given. */
	if(argc != 2)
	{
		SDL_LogError(PGBDBG_LOG_APPLICATION,
			"Usage: %s FILE\n", argv[0]);
		return EXIT_FAILURE;
	}

	/* Peanut-GB setup. */
	/* Copy input ROM file to allocated memory. */
	if((gb_priv.rom = read_rom_to_ram(argv[1])) == NULL)
	{
		SDL_LogError(PGBDBG_LOG_APPLICATION,
			"%d: %s\n", __LINE__, SDL_GetError());
		return EXIT_FAILURE;
	}

	/* Initialise Peanut-GB. */
	{
		size_t ram_sz;

		gb_init(&gb, gb_rom_read, gb_cart_ram_read, gb_cart_ram_write,
			gb_error, &gb_priv);

		gb_init_lcd(&gb, lcd_draw_line);
		ram_sz = gb_get_save_size(&gb);
		if(ram_sz != 0)
			gb_priv.ram = SDL_malloc(ram_sz);
		else
			gb_priv.ram = NULL;
	}

	/* SDL setup */
	set_dpi_awareness();
	SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "0");
	SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER) < 0)
	{
		SDL_LogError(PGBDBG_LOG_APPLICATION,
			"Error SDL_init: %s", SDL_GetError());
		SDL_free(gb_priv.rom);
		goto out;
	}

	win = SDL_CreateWindow("Debugger", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
		SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
	if(win == NULL)
	{
		SDL_LogError(PGBDBG_LOG_APPLICATION,
			"Error SDL_CreateWindow: %s", SDL_GetError());
		SDL_free(gb_priv.rom);
		SDL_Quit();
		goto out;
	}

	/* Set window title to ROM name. */
	{
		char title_str[64] = "Peanut-GB Debugger: ";
		gb_get_rom_name(&gb, &title_str[0] + strlen(title_str));
		SDL_SetWindowTitle(win, title_str);
	}

	renderer = SDL_CreateRenderer(win, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if(renderer == NULL)
	{
		SDL_LogError(PGBDBG_LOG_APPLICATION,
			"Error SDL_CreateRenderer: %s", SDL_GetError());
		SDL_free(gb_priv.rom);
		SDL_DestroyWindow(win);
		SDL_Quit();
		goto out;
	}

	/* Create textures for LCD and VRAM viewer. */
	{
		gb_priv.gb_lcd_tex = SDL_CreateTexture(renderer,
			SDL_PIXELFORMAT_RGBA32,
			SDL_TEXTUREACCESS_STREAMING,
			LCD_WIDTH, LCD_HEIGHT);
		SDL_assert_release(gb_priv.gb_lcd_tex != NULL);
		gb_priv.gb_vram_tex = SDL_CreateTexture(renderer,
			SDL_PIXELFORMAT_RGBA32,
			SDL_TEXTUREACCESS_STREAMING,
			VRAM_WIDTH, VRAM_HEIGHT);
		SDL_assert_release(gb_priv.gb_vram_tex != NULL);
	}

	/* Scale the renderer output for High-DPI displays */
	{
		int render_w, render_h;
		int window_w, window_h;
		float scale_x, scale_y;
		SDL_GetRendererOutputSize(renderer, &render_w, &render_h);
		SDL_GetWindowSize(win, &window_w, &window_h);
		scale_x = (float)(render_w) / (float)(window_w);
		scale_y = (float)(render_h) / (float)(window_h);
		SDL_RenderSetScale(renderer, scale_x, scale_y);
		font_scale = scale_y;
	}

	/* Initialise Nuklear GUI */
	ctx = nk_sdl_init(win, renderer);
	/* Load Fonts: if none of these are loaded a default font will be used  */
	/* Load Cursor: if you uncomment cursor loading please hide the cursor */
	{
		struct nk_font_atlas *atlas;
		struct nk_font_config config = nk_font_config(0);
		struct nk_font *font;

		/* set up the font atlas and add desired font; note that font sizes are
		 * multiplied by font_scale to produce better results at higher DPIs */
		nk_sdl_font_stash_begin(&atlas);
		font = nk_font_atlas_add_default(atlas, 13 * font_scale, &config);
		/*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/DroidSans.ttf", 14 * font_scale, &config);*/
		/*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Roboto-Regular.ttf", 16 * font_scale, &config);*/
		/*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13 * font_scale, &config);*/
		/*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12 * font_scale, &config);*/
		/*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10 * font_scale, &config);*/
		/*font = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13 * font_scale, &config);*/
		nk_sdl_font_stash_end();

		/* this hack makes the font appear to be scaled down to the desired
		 * size and is only necessary when font_scale > 1 */
		font->handle.height /= font_scale;
		/*nk_style_load_all_cursors(ctx, atlas->cursors);*/
		nk_style_set_font(ctx, &font->handle);
	}

#ifdef INCLUDE_STYLE
/*set_style(ctx, THEME_WHITE);*/
/*set_style(ctx, THEME_RED);*/
/*set_style(ctx, THEME_BLUE);*/
/*set_style(ctx, THEME_DARK);*/
#endif

	bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
	while(running)
	{
	    /* Input */
		SDL_Event evt;
		nk_input_begin(ctx);
		while(SDL_PollEvent(&evt))
		{
			switch(evt.type)
			{
			case SDL_QUIT:
				goto cleanup;

			case SDL_KEYDOWN:
				switch(evt.key.keysym.sym)
				{
				case SDLK_RETURN:
					gb.direct.joypad_bits.start = 0;
					break;

				case SDLK_BACKSPACE:
					gb.direct.joypad_bits.select = 0;
					break;

				case SDLK_z:
					gb.direct.joypad_bits.a = 0;
					break;

				case SDLK_x:
					gb.direct.joypad_bits.b = 0;
					break;

				case SDLK_UP:
					gb.direct.joypad_bits.up = 0;
					break;

				case SDLK_RIGHT:
					gb.direct.joypad_bits.right = 0;
					break;

				case SDLK_DOWN:
					gb.direct.joypad_bits.down = 0;
					break;

				case SDLK_LEFT:
					gb.direct.joypad_bits.left = 0;
					break;
				}
				break;
			case SDL_KEYUP:
				switch(evt.key.keysym.sym)
				{
				case SDLK_RETURN:
					gb.direct.joypad_bits.start = 1;
					break;

				case SDLK_BACKSPACE:
					gb.direct.joypad_bits.select = 1;
					break;

				case SDLK_z:
					gb.direct.joypad_bits.a = 1;
					break;

				case SDLK_x:
					gb.direct.joypad_bits.b = 1;
					break;

				case SDLK_UP:
					gb.direct.joypad_bits.up = 1;
					break;

				case SDLK_RIGHT:
					gb.direct.joypad_bits.right = 1;
					break;

				case SDLK_DOWN:
					gb.direct.joypad_bits.down = 1;
					break;

				case SDLK_LEFT:
					gb.direct.joypad_bits.left = 1;
					break;
				}
				break;
			}
			nk_sdl_handle_event(&evt);
		}
		nk_input_end(ctx);

		/* GUI */
		if(nk_begin(ctx, "Demo", nk_rect(710, 50, 230, 250),
			NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
			NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
		{
			enum {
				EASY, HARD
			};
			static int op = EASY;
			static int property = 20;

			nk_layout_row_static(ctx, 30, 80, 1);
			if(nk_button_label(ctx, "button"))
				SDL_Log("button pressed");
			nk_layout_row_dynamic(ctx, 30, 2);
			if(nk_option_label(ctx, "easy", op == EASY)) op = EASY;
			if(nk_option_label(ctx, "hard", op == HARD)) op = HARD;
			nk_layout_row_dynamic(ctx, 25, 1);
			nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

			nk_layout_row_dynamic(ctx, 20, 1);
			nk_label(ctx, "background:", NK_TEXT_LEFT);
			nk_layout_row_dynamic(ctx, 25, 1);
			if(nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx), 400)))
			{
				nk_layout_row_dynamic(ctx, 120, 1);
				bg = nk_color_picker(ctx, bg, NK_RGBA);
				nk_layout_row_dynamic(ctx, 25, 1);
				bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f, 0.005f);
				bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f, 0.005f);
				bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f, 0.005f);
				bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f, 0.005f);
				nk_combo_end(ctx);
			}
			print_window_pos(ctx);
		}
		nk_end(ctx);

		/* -------------- EXAMPLES ---------------- */
#ifdef INCLUDE_CALCULATOR
		calculator(ctx);
#endif
#ifdef INCLUDE_CANVAS
		canvas(ctx);
#endif
#ifdef INCLUDE_OVERVIEW
		overview(ctx);
#endif
#ifdef INCLUDE_NODE_EDITOR
		node_editor(ctx);
#endif
/* ----------------------------------------- */

		render_peanut_gb(ctx, &gb);
		SDL_SetRenderDrawColor(renderer, bg.r * 255, bg.g * 255, bg.b * 255, bg.a * 255);
		SDL_RenderClear(renderer);

		nk_sdl_render(NK_ANTI_ALIASING_ON);

		SDL_RenderPresent(renderer);
	}

cleanup:
	SDL_free(gb_priv.rom);
	SDL_free(gb_priv.ram);
	nk_sdl_shutdown();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(win);
	SDL_Quit();
	ret = EXIT_SUCCESS;

out:
	return ret;
}
