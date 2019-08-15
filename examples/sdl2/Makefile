.POSIX:
.SUFFIXES:
OPT = -g -Og
SDL2_CFLAGS = $(shell sdl2-config --cflags)
SDL2_LDLIBS =

# A check is performed later on as some Linux systems only install shared-lib
# variant of SDL2, so a static build isn't possible. This check is to stop
# building as soon as possible, instead of failing a build on the last step.
SDL2_ERRCHECK = 0
FILE_GUI_LIB = nativefiledialog/build/lib/Release/x64/libnfd.a
CWARNINGS = -Wall -Wextra
CXXWARNINGS = -Wall -Wextra

CC = cc
CFLAGS = -std=c99 $(SDL2_CFLAGS) $(OPT) $(CWARNINGS)
CXXFLAGS = $(SDL2_CFLAGS) $(OPT) $(CXXWARNINGS)
LDLIBS = -lm -L/usr/lib

ifeq ($(STATIC),yes)
	SDL2_LDLIBS = $(shell sdl2-config --static-libs)
	SDL2_ERRCHECK = $(.SHELLSTATUS)
	CFLAGS += -static
else
	SDL2_LDLIBS = $(shell sdl2-config --libs)
endif
LDLIBS += $(SDL2_LDLIBS)

# Enable sound by default
SOUND ?= peanut
ifeq ($(SOUND),blargg)
	SOUND_OBJECTS = blargg_apu/audio.o blargg_apu/Basic_Gb_Apu.o \
			blargg_apu/Blip_Buffer.o blargg_apu/Gb_Apu.o blargg_apu/Gb_Oscs.o \
			blargg_apu/Multi_Buffer.o
	# Enable sound support definitions within peanut_gb and peanut_sdl
	CFLAGS += -D ENABLE_SOUND -D ENABLE_SOUND_BLARGG
endif
ifeq ($(SOUND),peanut)
	SOUND_OBJECTS = peanut_apu/peanut_apu.o
	# Enable sound support definitions within peanut_gb and peanut_sdl
	CFLAGS += -D ENABLE_SOUND -D ENABLE_SOUND_PEANUT
endif

# Enable LCD by default
LCD ?= yes
ifeq ($(LCD),yes)
	CFLAGS += -D ENABLE_LCD=1
else
	CFLAGS += -D ENABLE_LCD=0
endif

# Enable file picker GUI by default
FILEGUI ?= yes
ifeq ($(FILEGUI),yes)
	# Object file defined above since it's platform independent.
	CFLAGS += -D ENABLE_FILE_GUI

# nativefiledialog has separate build procedures dependant on operating system.
# Support for Mac OS is ignored.
ifeq ($(OS),Windows_NT)
	LDLIBS += -lcomctl32 -lole32 -loleaut32 -luuid
	FILE_GUI_LIB = nativefiledialog/build/lib/Release/x64/nfd.lib
	NFD_MAKEFILE = nativefiledialog/build/gmake_windows
	# Enable static builds on Windows by default so that the application may
	# be distributed without DLL files.
	STATIC ?= yes
else
	LDLIBS += $(shell pkg-config --cflags --libs gtk+-3.0)
	NFD_MAKEFILE = nativefiledialog/build/gmake_linux
endif # OS Selection for FILEGUI

else
	# Reset lib path if we want to disable the file picker.
	FILE_GUI_LIB =
endif # FILEGUI


all: peanut-sdl
peanut-sdl: peanut_sdl.o $(SOUND_OBJECTS) $(FILE_GUI_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) 
peanut_sdl.o: sdl2_check peanut_sdl.c ../../peanut_gb.h \
	nativefiledialog/src/include/nfd.h

# Sound objects that are compiled when sound output is enabled.
blargg_apu/audio.o: blargg_apu/audio.cpp blargg_apu/audio.h blargg_apu/Basic_Gb_Apu.h \
	blargg_apu/Gb_Apu.h blargg_apu/Gb_Oscs.h blargg_apu/Blip_Buffer.h \
	blargg_apu/blargg_common.h blargg_apu/boost/config.hpp \
	blargg_apu/boost/cstdint.hpp blargg_apu/boost/static_assert.hpp \
	blargg_apu/Blip_Synth.h blargg_apu/Multi_Buffer.h
blargg_apu/Basic_Gb_Apu.o: blargg_apu/Basic_Gb_Apu.cpp blargg_apu/Basic_Gb_Apu.h \
	blargg_apu/Gb_Apu.h blargg_apu/Gb_Oscs.h blargg_apu/Blip_Buffer.h \
	blargg_apu/blargg_common.h blargg_apu/boost/config.hpp \
	blargg_apu/boost/cstdint.hpp blargg_apu/boost/static_assert.hpp \
	blargg_apu/Blip_Synth.h blargg_apu/Multi_Buffer.h
blargg_apu/Blip_Buffer.o: blargg_apu/Blip_Buffer.cpp blargg_apu/Blip_Buffer.h \
	blargg_apu/blargg_common.h blargg_apu/boost/config.hpp \
	blargg_apu/boost/cstdint.hpp blargg_apu/boost/static_assert.hpp \
	blargg_apu/Blip_Synth.h blargg_apu/blargg_source.h
blargg_apu/Gb_Apu.o: blargg_apu/Gb_Apu.cpp blargg_apu/Gb_Apu.h blargg_apu/Gb_Oscs.h \
	blargg_apu/Blip_Buffer.h blargg_apu/blargg_common.h blargg_apu/boost/config.hpp \
	blargg_apu/boost/cstdint.hpp blargg_apu/boost/static_assert.hpp \
	blargg_apu/Blip_Synth.h blargg_apu/blargg_source.h
blargg_apu/Gb_Oscs.o: blargg_apu/Gb_Oscs.cpp blargg_apu/Gb_Apu.h blargg_apu/Gb_Oscs.h \
	blargg_apu/Blip_Buffer.h blargg_apu/blargg_common.h \
	blargg_apu/boost/config.hpp blargg_apu/boost/cstdint.hpp \
	blargg_apu/boost/static_assert.hpp blargg_apu/Blip_Synth.h \
	blargg_apu/blargg_source.h
blargg_apu/Multi_Buffer.o: blargg_apu/Multi_Buffer.cpp blargg_apu/Multi_Buffer.h \
	blargg_apu/Blip_Buffer.h blargg_apu/blargg_common.h blargg_apu/boost/config.hpp \
	blargg_apu/boost/cstdint.hpp blargg_apu/boost/static_assert.hpp \
	blargg_apu/Blip_Synth.h blargg_apu/blargg_source.h

# File GUI picker.
$(FILE_GUI_LIB): nativefiledialog/src/include/nfd.h
	$(MAKE) nfd -C $(NFD_MAKEFILE)

sdl2_check:
ifneq (0,$(SDL2_ERRCHECK))
	$(error Error calling sdl2-config. Maybe --static-libs was not accepted)
endif

clean:
	rm -f peanut-sdl peanut_sdl.o $(SOUND_OBJECTS) $(FILE_GUI_LIB)

help:
	@echo Options:
	@echo \ STATIC=yes\	Enable static build. Default on Windows.
	@echo \	 	\	Requires that SDL2 be compiled with --static-libs enabled.
	@echo \ SOUND=peanut\	Sound output:
	@echo \	 	\	\	peanut \(default\)
	@echo \	 	\	\	blargg \(requires C++ compiler\)
	@echo \	 	\	\	none
	@echo \ FILEGUI=yes\	Enable graphical file picker. Default.
	@echo \	 	\	Requires C++ compiler.
	@echo
	@echo Values other than those specified will disable the option.
	@echo

.SUFFIXES: .c .cpp .o
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@
