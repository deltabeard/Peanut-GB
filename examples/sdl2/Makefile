NAME		:= Peanut-GB
DESCRIPTION	:= A DMG emulator using SDL2
COMPANY		:= Deltabeard
COPYRIGHT	:= Copyright (c) 2020 Mahyar Koshkouei
LICENSE_SPDX	:= MIT

# Available options:
SOUND := minigb
LCD := yes

# Default compiler options for GCC and Clang
CC	:= cc
OBJEXT	:= o
RM	:= rm -f
EXEOUT	:= -o
EXTRA_CFLAGS := -std=c99 -pedantic -Wall -Wextra -Og -g3
EXE	:= $(NAME)
LICENSE := $(COPYRIGHT); Released under the $(LICENSE_SPDX) License.
GIT_VER := $(shell git describe --dirty --always --tags --long)

TARGET := peanut-sdl
SRCS   := peanut_sdl.c
CFLAGS += $(shell sdl2-config --cflags)

ifeq ($(STATIC),yes)
	CFLAGS += -static
	LDLIBS += $(shell sdl2-config --static-libs)
else
	LDLIBS += $(shell sdl2-config --libs)
endif

ifeq ($(LCD),yes)
	CFLAGS += -D ENABLE_LCD=1
else
	CFLAGS += -D ENABLE_LCD=0
endif

ifeq ($(SOUND),minigb)
	SRCS += minigb_apu/minigb_apu.c
	CFLAGS += -D ENABLE_SOUND -D ENABLE_SOUND_MINIGB
	LDLIBS += -lm
endif

# File extension ".exe" is automatically appended on MinGW and MSVC builds, even
# if we don't ask for it.
ifeq ($(OS),Windows_NT)
	EXE := $(NAME).exe
endif

ifeq ($(GIT_VER),)
	GIT_VER := LOCAL
endif

CFLAGS += $(EXTRA_CFLAGS)

define help_txt
$(NAME) - $(DESCRIPTION)

Available options and their descriptions when enabled:
  STATIC=$(STATIC)
          Enable static build. Default on Windows.

  SOUND=$(SOUND)
          Sound processing library to use. One of:
	      minigb (default)
	      none (no audio will be played)

$(LICENSE)
endef

OBJS := $(SRCS:.c=.$(OBJEXT))
ifeq ($(CC),cl)
	OBJS += meta/winres.res
endif

all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(EXEOUT)$@ $^ $(LDFLAGS) $(LDLIBS) 

%.obj: %.c
	$(CC) $(CFLAGS) /Fo$@ /c /TC $^

%.res: %.rc
	rc /nologo /DCOMPANY="$(COMPANY)" /DDESCRIPTION="$(DESCRIPTION)" \
		/DLICENSE="$(LICENSE)" /DGIT_VER="$(GIT_VER)" \
		/DNAME="$(NAME)" /DICON_FILE="$(ICON_FILE)" $^

clean:
	$(RM) *.$(OBJEXT) $(TARGET)

help:
	@cd
	$(info $(help_txt))
