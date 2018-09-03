.POSIX:
CC		= cc
CFLAGS	= -g -Og -std=c99 -Wall -Wextra
LDLIBS	= -lSDL

all: gameboy_sdl
gameboy_sdl: ../gameboy.h gameboy_sdl.c
	$(CC) gameboy_sdl.c -o gameboy_sdl $(CFLAGS) $(LDLIBS)
clean:
	rm -f gameboy_sdl
