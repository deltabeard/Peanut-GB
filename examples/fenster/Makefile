CFLAGS := -O2 -g2
override CFLAGS += -Wall

ifeq ($(OS),Windows_NT)
	LDLIBS := -lgdi32 -lwinmm
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LDLIBS := -framework Cocoa
	else
		LDLIBS := -lX11
	endif
endif

all: peanut_fenster
