.POSIX:
CC	:= cc
OPT	:= -g3 -Og
CFLAGS	:= $(OPT) -std=c99 -Wall -Wextra -Wdouble-promotion \
     -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion \
     -fsanitize=undefined -fsanitize-trap
CFLAGS	+= $(shell sdl2-config --cflags)
LDLIBS	+= $(shell sdl2-config --libs)

all: peanut-debug

peanut-debug: peanut-debug.c ../../peanut_gb.h
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

clean:
	$(RM) peanut-debug
