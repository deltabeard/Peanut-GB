.POSIX:
CC		= cc
OPT		= -g -Og
CFLAGS	= $(OPT) -std=c99 -Wall -Wextra
LDLIBS	= -lSDL2

all: peanut-debug

peanut-debug: ../../peanut_gb.h peanut_debug.c
	$(CC) peanut_debug.c -o peanut-debug $(CFLAGS) $(LDLIBS)

clean:
	rm -f peanut-debug
