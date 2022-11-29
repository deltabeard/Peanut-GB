.POSIX:
CC		= cc
OPT		= -g -Og
CFLAGS	= $(OPT) -std=c99 -Wall -Wextra
LDLIBS	= -lSDL2

all: peanut-debug

peanut-debug: peanut-debug.c ../../peanut_gb.h
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

clean:
	$(RM) peanut-debug
