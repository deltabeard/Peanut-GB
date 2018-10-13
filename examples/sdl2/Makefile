.POSIX:
CC		= cc
OPT		= -s -Ofast
CFLAGS	= $(OPT) -std=c99 -Wall -Wextra
LDLIBS	= -lSDL2

all: peanut-sdl

peanut-sdl: ../../peanut_gb.h peanut_sdl.c
	$(CC) peanut_sdl.c -o peanut-sdl $(CFLAGS) $(LDLIBS)

clean:
	rm -f peanut-sdl
