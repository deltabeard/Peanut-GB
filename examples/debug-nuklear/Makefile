# Simple Makefile for building peanutgb-debugger on Unix-like systems.
# SDL2 is required.

ifeq ($(OS),Windows_NT)
$(warning This Makefile is only support with MinGW on Windows; otherwise use cmake instead)
endif

ifneq ($(W64DEVKIT),)
# w64devkit uses pkg-config out of the box.
SDL2_CONFIG := pkg-config sdl2
endif

SDL2_CONFIG ?= sdl2-config
SDL2_CFLAGS := $(shell $(SDL2_CONFIG) --cflags)
SDL2_LDLIBS := $(shell $(SDL2_CONFIG) --libs)
CFLAGS := -std=c99 -Wall -Wextra -Og -g3

override CFLAGS += -Iinc $(SDL2_CFLAGS)
override LDLIBS += $(SDL2_LDLIBS)

all: peanutgb-debugger
peanutgb-debugger: src/main.o src/nuklear.o src/overview.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)
