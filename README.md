# Peanut-GB

Peanut-GB is a single file header Game Boy emulator library based off of [this
gameboy emulator](https://github.com/gregtour/gameboy). The aim is to make a
high speed and portable Game Boy (DMG) emulator library that may be used for any
platform that has a C99 compiler.

Check out [BENCHMARK.md](BENCHMARK.md) for benchmarks of Peanut-GB.

Only the original Game Boy (DMG) is supported at this time.

This emulator is inaccurate and is very much a work in progress (although it
does pass Blargg's CPU instructions test). As such, some games may run
incorrectly or not run at all. High speed is important; changes that cause a
significant loss of emulation speed but increase in accuracy, will be rejected.
Please seek an alternative emulator if accuracy is important.

## SDL2 Example

An example implementation is given in peanut_sdl.c, which uses SDL2 to draw the
screen and take input. Run `cmake` or `make` in the ./examples/sdl2/ folder to
compile it.

Execute in command line with `peanut-sdl game.gb` which will automatically
create the save file `game.sav` for the game if one isn't found. Or run with
`peanut-sdl game.gb save.sav` to specify a save file. Or even `peanut-sdl` to
create a *drop zone* window that you can drag and drop a ROM file to.

### Screenshot

![Pokemon Blue - Main screen animation](/screencaps/PKMN_BLUE.gif)
![Legend of Zelda: Links Awakening - animation](/screencaps/ZELDA.gif)
![Megaman V](/screencaps/MEGAMANV.png)

![Shantae](/screencaps/SHANTAE.png)
![Dragon Ball Z](/screencaps/DRAGONBALL_BBZP.png)

Note: Animated GIFs shown here are limited to 50fps, whilst the emulation was
running at the native ~60fps. This is because popular GIF decoders limit the
maximum FPS to 50.

### Controls

| Action            | Keyboard   | Joypad |
|-------------------|------------|--------|
| A                 | z          | A      |
| B                 | x          | B      |
| Start             | Return     | START  |
| Select            | Backspace  | BACK   |
| D-Pad             | Arrow Keys | DPAD   |
| Normal Speed      | 1          |        |
| Turbo x2 (Hold)   | Space      |        |
| Turbo X2 (Toggle) | 2          |        |
| Turbo X3 (Toggle) | 3          |        |
| Turbo X4 (Toggle) | 4          |        |
| Reset             | r          |        |
| Change Palette    | p          |        |
| Reset Palette     | Shift + p  |        |
| Fullscreen        | F11 / f    |        |
| Frameskip (Toggle)| o          |        |
| Interlace (Toggle)| i          |        |
| Dump BMP (Toggle) | b          |        |

Frameskip and Interlaced modes are both off by default. The Frameskip toggles
between 60 FPS and 30 FPS.

Pressing 'b' will dump each frame as a 24-bit bitmap file in the current
folder. See /screencaps/README.md for more information.

## Getting Started

Documentation of function prototypes can be found at the bottom of [peanut_gb.h](peanut_gb.h#L3960).

### Required Functions

The front-end implementation must provide a number of functions to the library.
These functions are set when calling gb_init.

- gb_rom_read
- gb_cart_ram_read
- gb_cart_ram_write
- gb_error

### Optional Functions

The following optional functions may be defined for further functionality.

#### lcd_draw_line

This function is required for LCD drawing. Set this function using gb_init_lcd
and enable LCD functionality within Peanut-GB by defining ENABLE_LCD to 1 before
including peanut_gb.h. ENABLE_LCD is set to 1 by default if it was not
previously defined. If gb_init_lcd is not called or lcd_draw_line is set to
NULL, then LCD drawing is disabled.

The pixel data sent to lcd_draw_line comes with both shade and layer data. The
first two least significant bits are the shade data (black, dark, light, white).
Bits 4 and 5 are layer data (OBJ0, OBJ1, BG), which can be used to add more
colours to the game in the same way that the Game Boy Color does to older Game
Boy games.

#### audio_read and audio_write

These functions are required for audio emulation and output. Peanut-GB does not
include audio emulation, so an external library must be used. These functions
must be defined and audio output must be enabled by defining ENABLE_SOUND to 1
before including peanut_gb.h. 

#### gb_serial_tx and gb_serial_rx

These functions are required for serial communication. Set these functions using
gb_init_serial. If these functions are not set, then the emulation will act as
though no link cable is connected.

### Useful Functions

These functions are provided by Peanut-GB.

#### gb_reset

This function resets the game being played, as though the console had been
powered off and on. gb_reset is called by gb_init to initialise the CPU
registers.

#### gb_get_save_size

This function returns the save size of the game being played. This function
returns 0 if the game does not use any save data.

#### gb_run_frame

This function runs the CPU until a full frame is rendered to the LCD.

#### gb_color_hash

This function calculates a hash of the game title. This hash is calculated in
the same way as the Game Boy Color to add colour to Game Boy games.

#### gb_get_rom_name

This function returns the name of the game.

#### gb_set_rtc

Set the time of the real time clock (RTC). Some games use this RTC data.

#### gb_tick_rtc

Increment the real time clock by one second.

#### gb_set_bios

Execute a bootrom image on reset. A reset must be performed after calling
gb_set_bios for these changes to take effect. This is because gb_init calls
gb_reset, but gb_set_bios must be called after gb_init.
The bootrom must be either a DMG or a MGB bootrom.

## License

This project is licensed under the MIT License.
