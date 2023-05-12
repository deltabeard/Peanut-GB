OPT := -O2 -g2
CFLAGS := -Wdouble-promotion -fsanitize=undefined -fsanitize-trap
override CFLAGS += -Wall -Wextra $(OPT)

ifeq ($(OS),Windows_NT)
	LDLIBS := -lgdi32 -lwinmm -luser32
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LDLIBS := -framework Cocoa
	else
		LDLIBS := -lX11
	endif
endif

all: peanut_fenster
