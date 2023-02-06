NAME		:= Peanut-GB
DESCRIPTION	:= A DMG emulator using SDL2
COMPANY		:= Deltabeard
COPYRIGHT	:= Copyright (c) 2020 Mahyar Koshkouei
LICENSE_SPDX	:= MIT


CPPFLAGS := -DCOMPANY=Deltabeard		\
	-DDESCRIPTION="$(DESCRIPTION)"		\
	-DLICENSE="$(LICENSE_SPDX)"		\
	-DNAME="$(NAME)"			\
	-DICON_FILE=./meta/icon.ico		\
	-DENABLE_SOUND -DENABLE_SOUND_MINIGB

OPT := -O2 -Wall -Wextra
CFLAGS := $(OPT) $(shell sdl2-config --cflags)
LDLIBS := $(shell sdl2-config --libs)

SOURCES := peanut_sdl.c minigb_apu/minigb_apu.c
OBJECTS := peanut_sdl.o minigb_apu/minigb_apu.o

ifeq ($(OS),Windows_NT)
	OBJECTS += meta/winres.o
endif

all: peanut-sdl
peanut-sdl: $(OBJECTS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDLIBS) -o $@ $^

meta/winres.o: meta/winres.rc
	windres $(CPPFLAGS) $< $@
