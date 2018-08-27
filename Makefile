.POSIX:
CC		= cc
CFLAGS	= -g -Og -std=c99 -Wall

all: gameboy
gameboy: gameboy.h example.c
	$(CC) platform.c -o gameboy $(CFLAGS)
clean:
	rm -f gameboy
