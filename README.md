Game Boy Emulator Single File Library in C
==========================================

Presented is a single file header Game Boy emulator library based off of [this
gameboy emulator](https://github.com/gregtour/gameboy). The aim is to make a
portable implementation that may be used for any platform that has a C99
compiler.

Only the original Game Boy (DMG) is supported at this time.

The front-end implementation must provide a number of functions to the library.
These are:

- gb_rom_read
- gb_cart_ram_read
- gb_cart_ram_write
- gb_error

An example implementation is given in gameboy_sdl.c, which uses SDL2 to draw the
screen. Run `make` in the example folder to compile.
