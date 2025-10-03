# Peanut-GB

Peanut-GB is a single file header Game Boy emulator library based off of [this
gameboy emulator](https://github.com/gregtour/gameboy). The aim is to make a
high speed and portable Game Boy (DMG) emulator library that may be used for any
platform that has a C99 compiler.

This emulator is *very fast*. So much so that it can run at
[full speed on the Raspberry Pi Pico](https://github.com/deltabeard/RP2040-GB)!

Only the original Game Boy (DMG) is supported at this time, but preliminary work
has been completed to support Game Boy Color
(see https://github.com/deltabeard/Peanut-GB/issues/50).

This emulator is a work in progress and can be inaccurate (although it does pass
Blargg's CPU instructions and instruction timing tests). As such, some games may
run incorrectly or not run at all. Please seek an alternative emulator if
accuracy is important.

## Features

- Game Boy (DMG) Support
- Very fast; fast enough to run on a RP2040 ARM Cortex M0+ microcontroller at
  full speed.
- MBC1, MBC2, MBC3, and MBC5 support
- Real Time Clock (RTC) support
- Serial connection support
- Can be used with or without a bootrom
- Allows different palettes on background and sprites
- Frame skip and interlacing modes (useful for slow LCDs)
- Simple to use and comes with examples
- LCD and sound can be disabled at compile time.
- If sound is enabled, an external audio processing unit (APU) library is
  required.
  A fast audio processing unit (APU) library is included in this repository at
  https://github.com/deltabeard/Peanut-GB/tree/master/examples/sdl2/minigb_apu .

## Caveats

- The LCD rendering is performed line by line, so certain animations will not
  render properly (such as in Prehistorik Man).
- Some games may not be playable due to emulation inaccuracy
  (see https://github.com/deltabeard/Peanut-GB/issues/31).
- MiniGB APU runs in a separate thread, and so the timing is not accurate. If
  accurate APU timing and emulation is required, then Blargg's Gb_Snd_Emu
  library (or an alternative) can be used instead.

## SDL2 Example

The flagship example implementation is given in peanut_sdl.c, which uses SDL2 to
draw the screen and take input. Run `cmake` or `make` in the ./examples/sdl2/
folder to compile it.

Run `peanut-sdl`, which creates a *drop-zone* window that you can drag and drop
a ROM file into.  Alternatively, run in a terminal using `peanut-sdl game.gb`,
which will automatically create the save file `game.sav` for the game if one
isn't found. Or, run with `peanut-sdl game.gb save.sav` to specify a save file.

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
| Repeat A          | a          |        |
| Repeat B          | s          |        |
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

## Projects Using Peanut-GB

In no particular order, and a non-exhaustive list, the following projects use Peanut-GB.

* [Pico-GB](https://github.com/YouMakeTech/Pico-GB) -  Game Boy emulation on the Raspberry Pi RP2040 microcontroller.
* [Peanut_gb-RGFW](https://github.com/ColleagueRiley/Peanut_gb-RGFW) - A Gameboy emulator example for [RGFW](https://github.com/ColleagueRiley/RGFW).
* [CPBoy](https://github.com/diddyholz/CPBoy) - A Game Boy emulator for the Classpad II (fx-CP400).
* [PlayGB](https://github.com/risolvipro/PlayGB) - A Game Boy emulator for Playdate.
* [AcolyteHandPICd32](https://github.com/stevenchadburrow/AcolyteHandPICd32) - Game Boy emulation on the PIC32MZ2048EFH144 32-bit 150MHz microcontroller.

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

#### gb_colour_hash

This function calculates a hash of the game title. This hash is calculated in
the same way as the Game Boy Color to add colour to Game Boy games.

#### gb_get_rom_name

This function returns the name of the game.

#### gb_set_rtc

Set the time of the real time clock (RTC). Some games use this RTC data.

#### gb_tick_rtc

Deprecated: do not use. The RTC is ticked internally.

#### gb_set_bootrom

Execute a bootrom image on reset. A reset must be performed after calling
gb_set_bootrom for these changes to take effect. This is because gb_init calls
gb_reset, but gb_set_bootrom must be called after gb_init.
The bootrom must be either a DMG or a MGB bootrom.

## License

This project is licensed under the MIT License.
