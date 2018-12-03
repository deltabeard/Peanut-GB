.POSIX:
.SUFFIXES:
OPTIMIZE_FLAG = -s -Ofast
SDL2_CFLAGS = $(shell sdl2-config --cflags)
FILE_GUI_LIB = nativefiledialog/build/lib/Release/x64/libnfd.a
CWARNINGS = -Wall -Wextra
# Not interested in the many warnings in blarggs APU library, so not enabling
# warnings for CXX targets.
CXXWARNINGS = -w

CC = cc
CFLAGS = -std=c99 $(SDL2_CFLAGS) $(OPTIMIZE_FLAG) $(CWARNINGS)
CXXFLAGS = $(SDL2_CFLAGS) $(OPTIMIZE_FLAG) $(CXXWARNINGS)
LDLIBS = -lm

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
endif

ifeq ($(STATIC),yes)
	LDLIBS += $(shell sdl2-config --libs --static-libs)
	CFLAGS += -static
else
	LDLIBS += $(shell sdl2-config --libs)
endif

all: peanut-sdl
peanut-sdl: peanut_sdl.o gb_apu/audio.o gb_apu/Basic_Gb_Apu.o \
	gb_apu/Blip_Buffer.o gb_apu/Gb_Apu.o gb_apu/Gb_Oscs.o \
	gb_apu/Multi_Buffer.o $(FILE_GUI_LIB)
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) 
peanut_sdl.o: peanut_sdl.c ../../peanut_gb.h \
	nativefiledialog/src/include/nfd.h

gb_apu/audio.o: gb_apu/audio.cpp gb_apu/audio.h gb_apu/Basic_Gb_Apu.h \
	gb_apu/Gb_Apu.h gb_apu/Gb_Oscs.h gb_apu/Blip_Buffer.h \
	gb_apu/blargg_common.h gb_apu/boost/config.hpp \
	gb_apu/boost/cstdint.hpp gb_apu/boost/static_assert.hpp \
	gb_apu/Blip_Synth.h gb_apu/Multi_Buffer.h
gb_apu/Basic_Gb_Apu.o: gb_apu/Basic_Gb_Apu.cpp gb_apu/Basic_Gb_Apu.h \
	gb_apu/Gb_Apu.h gb_apu/Gb_Oscs.h gb_apu/Blip_Buffer.h \
	gb_apu/blargg_common.h gb_apu/boost/config.hpp \
	gb_apu/boost/cstdint.hpp gb_apu/boost/static_assert.hpp \
	gb_apu/Blip_Synth.h gb_apu/Multi_Buffer.h
gb_apu/Blip_Buffer.o: gb_apu/Blip_Buffer.cpp gb_apu/Blip_Buffer.h \
	gb_apu/blargg_common.h gb_apu/boost/config.hpp \
	gb_apu/boost/cstdint.hpp gb_apu/boost/static_assert.hpp \
	gb_apu/Blip_Synth.h gb_apu/blargg_source.h
gb_apu/Gb_Apu.o: gb_apu/Gb_Apu.cpp gb_apu/Gb_Apu.h gb_apu/Gb_Oscs.h \
	gb_apu/Blip_Buffer.h gb_apu/blargg_common.h gb_apu/boost/config.hpp \
	gb_apu/boost/cstdint.hpp gb_apu/boost/static_assert.hpp \
	gb_apu/Blip_Synth.h gb_apu/blargg_source.h
gb_apu/Gb_Oscs.o: gb_apu/Gb_Oscs.cpp gb_apu/Gb_Apu.h gb_apu/Gb_Oscs.h \
	gb_apu/Blip_Buffer.h gb_apu/blargg_common.h \
	gb_apu/boost/config.hpp gb_apu/boost/cstdint.hpp \
	gb_apu/boost/static_assert.hpp gb_apu/Blip_Synth.h \
	gb_apu/blargg_source.h
gb_apu/Multi_Buffer.o: gb_apu/Multi_Buffer.cpp gb_apu/Multi_Buffer.h \
	gb_apu/Blip_Buffer.h gb_apu/blargg_common.h gb_apu/boost/config.hpp \
	gb_apu/boost/cstdint.hpp gb_apu/boost/static_assert.hpp \
	gb_apu/Blip_Synth.h gb_apu/blargg_source.h
$(FILE_GUI_LIB): nativefiledialog/src/include/nfd.h
	$(MAKE) nfd -C $(NFD_MAKEFILE)

clean:
	rm -f peanut-sdl peanut_sdl.o gb_apu/audio.o gb_apu/Basic_Gb_Apu.o \
		gb_apu/Blip_Buffer.o gb_apu/Gb_Apu.o gb_apu/Gb_Oscs.o \
		gb_apu/Multi_Buffer.o $(FILE_GUI_LIB)

help:
	@echo Options:
	@echo STATIC=yes	Enable static build. Default on Windows.

.SUFFIXES: .c .cpp .o
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@
