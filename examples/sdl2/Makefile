NAME		:= Peanut-GB
DESCRIPTION	:= A DMG emulator using SDL2
COMPANY		:= Deltabeard
COPYRIGHT	:= Copyright (c) 2020 Mahyar Koshkouei
LICENSE_SPDX	:= MIT

SDL2_PREFIX :=

CPPFLAGS := -DCOMPANY=Deltabeard		\
	-DDESCRIPTION="$(DESCRIPTION)"		\
	-DLICENSE="$(LICENSE_SPDX)"		\
	-DNAME="$(NAME)"			\
	-DICON_FILE=./meta/icon.ico

OPT := -O2 -Wall -Wextra
CFLAGS := $(OPT) $(shell sdl2-config --prefix="$(SDL2_PREFIX)" --cflags)
LDLIBS := $(shell sdl2-config --prefix="$(SDL2_PREFIX)" --libs)

SOURCES := peanut_sdl.c
OBJECTS := peanut_sdl.o

ifeq ($(OS),Windows_NT)
	OBJECTS += meta/winres.o
endif

all: peanut-sdl
peanut-sdl: $(OBJECTS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDLIBS) -o $@ $^

meta/winres.o: meta/winres.rc
	windres $(CPPFLAGS) $< $@
