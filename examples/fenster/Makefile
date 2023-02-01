CFLAGS := -O2 -g2
override CFLAGS += -Wall

ifeq ($(OS),Windows_NT)
	LDFLAGS := -lgdi32 -lwinmm
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LDFLAGS = -framework Cocoa
	else
		LDFLAGS = -lX11
	endif
endif

all: peanut_fenster
peanut_fenster: peanut_fenster.c
	$(CC) $< -o $@ $(CFLAGS) $(LDFLAGS)
