.POSIX:
.SUFFIXES:
OPTIMIZE_FLAG = -s -Ofast
SDL2_CFLAGS = $(shell sdl2-config --cflags)
SDL2_LIBS = $(shell sdl2-config --libs)
CWARNINGS = -W -Wall -Wextra
# Not interested in the many warnings in blarggs APU library, so not enabling
# warnings for CXX targets.
CXXWARNINGS = -W

CC = cc
CFLAGS = -std=c99 $(SDL2_CFLAGS) $(OPTIMIZE_FLAG) $(CWARNINGS)
CXXFLAGS = $(SDL2_CFLAGS) $(OPTIMIZE_FLAG) $(CXXWARNINGS)
LDLIBS = -lm $(SDL2_LIBS) $(FILE_DIALOG)

ifeq ($(OS),Windows_NT)
	LDLIBS += -lcomctl32.lib
else
	LDLIBS += $(shell pkg-config --cflags --libs gtk+-3.0)
endif

all: peanut-sdl
peanut-sdl: peanut_sdl.o gb_apu/audio.o gb_apu/Basic_Gb_Apu.o \
	gb_apu/Blip_Buffer.o gb_apu/Gb_Apu.o gb_apu/Gb_Oscs.o \
	gb_apu/Multi_Buffer.o nativefiledialog/build/lib/Release/x64/libnfd.a
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

clean:
	rm -f peanut-sdl peanut_sdl.o gb_apu/audio.o gb_apu/Basic_Gb_Apu.o \
		gb_apu/Blip_Buffer.o gb_apu/Gb_Apu.o gb_apu/Gb_Oscs.o \
		gb_apu/Multi_Buffer.o

.SUFFIXES: .c .cpp .o
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@
