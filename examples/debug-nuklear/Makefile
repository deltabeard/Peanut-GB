# Simple Makefile for building on Unix-like systems.
# SDL2 is required.

ifeq ($(OS),Windows_NT)
$(error This Makefile is not supported on Windows; use cmake instead)
endif

SDL2_CFLAGS := $(shell sdl2-config --cflags)
SDL2_LDLIBS := $(shell sdl2-config --libs)
CFLAGS := -std=c99 -Wall -Wextra -Og -g3

override CFLAGS += -Iinc $(SDL2_CFLAGS)
override LDLIBS += $(SDL2_LDLIBS)

all: peanutgb-debugger
peanutgb-debugger: src/main.o src/nuklear.o src/overview.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)
