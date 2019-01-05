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

## Getting Started

The front-end implementation must provide a number of functions to the library.
These are:

- gb_rom_read
- gb_cart_ram_read
- gb_cart_ram_write
- gb_serial_transfer
- gb_error

## SDL2 Example

An example implementation is given in peanut_sdl.c, which uses SDL2 to draw the
screen and take input. Run `make` in the ./examples/sdl2/ folder to compile it.

Execute in command line with `peanut-sdl game.gb` which will automatically
create the save file `game.sav` for the game if one isn't found. Or run with
`peanut-sdl game.gb save.sav` to specify a save file. Or even `peanut-sdl` to
use a file picker native to your running operating system to select the ROM.

### Screenshot

![Screenshot of SDL2 frontend](/examples/sdl2/sdl2_example_screenshot.png)

### Controls

| Action            | Key        |
|-------------------|------------|
| A                 | z          |
| B                 | x          |
| Start             | Return     |
| Select            | Backspace  |
| D-Pad             | Arrow Keys |
| Turbo (Hold)      | Space      |
| Normal Speed      | 1          |
| Turbo X2 (Toggle) | 2          |
| Turbo X3 (Toggle) | 3          |
| Turbo X4 (Toggle) | 4          |
| Reset             | r          |
| Change Palette    | p          |
| Reset Palette     | Shift + p  |
| Fullscreen        | F11        |
| Frameskip (Toggle)| f          |
| Interlace (Toggle)| i          |

Frameskip and Interlaced modes are both off by default. The Frameskip toggles
between 60 FPS and 30 FPS.

## License

This project is licensed under the MIT License.
