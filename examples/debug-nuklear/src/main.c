/* nuklear - 1.32.0 - public domain */

#include <stdio.h>
#include <SDL.h>
#define ENABLE_LCD 1
#include "../../../peanut_gb.h"
#include "nuklear_proj.h"
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "nuklear_sdl_renderer.h"

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

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

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# include <Windows.h>

# if (_WIN32_WINNT >= 0x0603)
#  include <shellscalingapi.h>
# endif

static SDL_Renderer *renderer;
typedef struct {
	SDL_Texture *gb_lcd_tex;
	void *pixels;
	int pitch;
} gb_priv_s;
static uint8_t *rom;
static uint8_t *ram;

static uint8_t gb_rom_read(struct gb_s *ctx, const uint_fast32_t addr)
{
	return rom[addr];
}

static uint8_t gb_cart_ram_read(struct gb_s *ctx, const uint_fast32_t addr)
{
	return ram[addr];
}

static void gb_cart_ram_write(struct gb_s *ctx, const uint_fast32_t addr,
		const uint8_t val)
{
	ram[addr] = val;
}

static void gb_error(struct gb_s *ctx, const enum gb_error_e err,
		const uint16_t val)
{
	const char *err_str[] = {
		"Unknown", "Invalid opcode", "Invalid read", "Invalid write",
		"Halted forever"
	};
	SDL_Log("Error: %s", err_str[err]);
	SDL_assert_release(0);
}

static void lcd_draw_line(struct gb_s *gb, const uint8_t *pixels,
		const uint_fast8_t line)
{
	const uint32_t colour_lut[4] = {
		0xFFFFFFFF, 0x7F7F7FFF, 0x2F2F2FFF, 0
	};
	gb_priv_s *priv = gb->direct.priv;
	uint8_t *tex = priv->pixels;

	for(unsigned int x = 0; x < LCD_WIDTH; x++)
	{
		tex[line + x] = colour_lut[pixels[x] & 3];
	}

	return;
}

static void render_peanut_gb(struct nk_context *ctx)
{
	static int start = 1;
	static struct gb_s gb;
	static char title_str[16] = "No Game";
	static gb_priv_s gb_priv;

	if(start)
	{
		size_t ram_sz;

		start = 0;
		gb_init(&gb, gb_rom_read, gb_cart_ram_read, gb_cart_ram_write,
				gb_error, &gb_priv);

		gb_init_lcd(&gb, lcd_draw_line);
		gb_priv.gb_lcd_tex = SDL_CreateTexture(renderer,
				SDL_PIXELFORMAT_RGBA32,
				SDL_TEXTUREACCESS_STREAMING,
				LCD_WIDTH, LCD_HEIGHT);
		SDL_assert_release(gb_priv.gb_lcd_tex != NULL);

		gb_get_rom_name(&gb, title_str);
		ram_sz = gb_get_save_size(&gb);
		if(ram_sz != 0)
		{
			ram = SDL_malloc(ram_sz);
		}
	}

	SDL_assert_always(SDL_LockTexture(gb_priv.gb_lcd_tex,
		NULL, &gb_priv.pixels, &gb_priv.pitch) == 0);
	gb_run_frame(&gb);
	SDL_UnlockTexture(gb_priv.gb_lcd_tex);

	/* GUI */
	if (nk_begin(ctx, title_str,
				nk_rect(50, 50, 50 + LCD_WIDTH, 50 + LCD_HEIGHT),
				NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_TITLE))
	{
		struct nk_rect emu_lcd;
		SDL_FRect targ_rect;

		emu_lcd = nk_window_get_content_region(ctx);
		targ_rect.h = emu_lcd.h;
		targ_rect.w = emu_lcd.w;
		targ_rect.x = emu_lcd.x;
		targ_rect.y = emu_lcd.y;
		SDL_RenderCopyF(renderer, gb_priv.gb_lcd_tex, NULL, &targ_rect);
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

	if(fread(rom, sizeof(uint8_t), rom_size, rom_file) != rom_size)
	{
		free(rom);
		fclose(rom_file);
		return NULL;
	}

	fclose(rom_file);
	return rom;
}

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
#endif

/* ===============================================================
 *
 *                          DEMO
 *
 * ===============================================================*/
int
main(int argc, char *argv[])
{
    /* Platform */
    SDL_Window *win;
    int running = 1;
    float font_scale = 1;

    /* GUI */
    struct nk_context *ctx;
    struct nk_colorf bg;

    set_dpi_awareness();

    /* Make sure a file name is given. */
    if(argc != 2)
    {
	    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
		    "Usage: %s FILE [SAVE]\n", argv[0]);
	    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
		    "SAVE is set by default if not provided.");
	    return EXIT_FAILURE;
    }

    /* Copy input ROM file to allocated memory. */
    if((rom = read_rom_to_ram(argv[1])) == NULL)
    {
	    SDL_Log("%d: %s\n", __LINE__, strerror(errno));
	    return EXIT_FAILURE;
    }

    /* SDL setup */
    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
    SDL_Init(SDL_INIT_VIDEO);

    win = SDL_CreateWindow("Demo", SDL_WINDOWPOS_CENTERED,
	    SDL_WINDOWPOS_CENTERED,WINDOW_WIDTH, WINDOW_HEIGHT,
	    SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

    if (win == NULL) {
        SDL_Log("Error SDL_CreateWindow %s", SDL_GetError());
        exit(-1);
    }

#if 0
    SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
#endif

    renderer = SDL_CreateRenderer(win, -1,
		    SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        SDL_Log("Error SDL_CreateRenderer %s", SDL_GetError());
        exit(-1);
    }

    /* scale the renderer output for High-DPI displays */
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

    /* GUI */
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
    while (running)
    {
        /* Input */
        SDL_Event evt;
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) goto cleanup;
            nk_sdl_handle_event(&evt);
        }
        nk_input_end(ctx);

        /* GUI */
        if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
            NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
            NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
        {
            enum {EASY, HARD};
            static int op = EASY;
            static int property = 20;

            nk_layout_row_static(ctx, 30, 80, 1);
            if (nk_button_label(ctx, "button"))
                SDL_Log("button pressed");
            nk_layout_row_dynamic(ctx, 30, 2);
            if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
            if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;
            nk_layout_row_dynamic(ctx, 25, 1);
            nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

            nk_layout_row_dynamic(ctx, 20, 1);
            nk_label(ctx, "background:", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 25, 1);
            if (nk_combo_begin_color(ctx, nk_rgb_cf(bg), nk_vec2(nk_widget_width(ctx),400))) {
                nk_layout_row_dynamic(ctx, 120, 1);
                bg = nk_color_picker(ctx, bg, NK_RGBA);
                nk_layout_row_dynamic(ctx, 25, 1);
                bg.r = nk_propertyf(ctx, "#R:", 0, bg.r, 1.0f, 0.01f,0.005f);
                bg.g = nk_propertyf(ctx, "#G:", 0, bg.g, 1.0f, 0.01f,0.005f);
                bg.b = nk_propertyf(ctx, "#B:", 0, bg.b, 1.0f, 0.01f,0.005f);
                bg.a = nk_propertyf(ctx, "#A:", 0, bg.a, 1.0f, 0.01f,0.005f);
                nk_combo_end(ctx);
            }
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

	render_peanut_gb(ctx);
        SDL_SetRenderDrawColor(renderer, bg.r * 255, bg.g * 255, bg.b * 255, bg.a * 255);
        SDL_RenderClear(renderer);

        nk_sdl_render(NK_ANTI_ALIASING_ON);

        SDL_RenderPresent(renderer);
    }

cleanup:
    nk_sdl_shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
