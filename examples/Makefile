.POSIX:
CC		= cc
OPT		= -g -Og
CFLAGS	= $(OPT) -std=c99 -Wall -Wextra
LDLIBS	= -lSDL

all: gameboy_sdl

gameboy_sdl: ../gameboy.h gameboy_sdl.c
	$(CC) gameboy_sdl.c -o gameboy_sdl $(CFLAGS) $(LDLIBS)

gameboy_debug: ../gameboy.h gameboy_debug.c
	$(CC) gameboy_debug.c -o gameboy_debug $(CFLAGS) $(LDLIBS)

clean:
	rm -f gameboy_sdl gameboy_debug
