.POSIX:
CC		= cc
OPT		= -g -Og
CFLAGS	= $(OPT) -std=c99 -Wall -Wextra
LDLIBS	= -lSDL2

all: peanut-sdl

peanut-sdl: ../peanut_gb.h peanut_sdl.c
	$(CC) peanut_sdl.c -o peanut-sdl $(CFLAGS) $(LDLIBS)

peanut-debug: ../peanut-gb.h peanut_debug.c
	$(CC) peanut_debug.c -o peanut-debug $(CFLAGS) $(LDLIBS)

clean:
	rm -f peanut-sdl peanut-debug
