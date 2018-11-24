.POSIX:
.SUFFIXES:
OPTIMIZE_FLAG = -s -Os
SDL2_CFLAGS = $(shell sdl2-config --cflags)
SDL2_LIBS = $(shell sdl2-config --libs)
CWARNINGS = -W -Wall -Wextra
# Not interested in the many warnings in blarggs APU library, so not enabling
# warnings for CXX targets.
CXXWARNINGS = -W

CC = cc
CFLAGS = -std=c99 $(SDL2_CFLAGS) $(OPTIMIZE_FLAG) $(CWARNINGS)
CXXFLAGS = $(SDL2_CFLAGS) $(OPTIMIZE_FLAG) $(CXXWARNINGS)
LDLIBS = -lm $(SDL2_LIBS)

all: peanut-sdl
peanut-sdl: peanut_sdl.o audio.o Basic_Gb_Apu.o gb_apu/Blip_Buffer.o \
	gb_apu/Gb_Apu.o gb_apu/Gb_Oscs.o gb_apu/Multi_Buffer.o
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) 
peanut_sdl.o: peanut_sdl.c ../../peanut_gb.h

audio.o: audio.cpp audio.h Basic_Gb_Apu.h gb_apu/Gb_Apu.h gb_apu/Gb_Oscs.h \
	gb_apu/Blip_Buffer.h gb_apu/blargg_common.h gb_apu/boost/config.hpp \
	gb_apu/boost/cstdint.hpp gb_apu/boost/static_assert.hpp \
	gb_apu/Blip_Synth.h gb_apu/Multi_Buffer.h
Basic_Gb_Apu.o: Basic_Gb_Apu.cpp Basic_Gb_Apu.h gb_apu/Gb_Apu.h \
	gb_apu/Gb_Oscs.h gb_apu/Blip_Buffer.h gb_apu/blargg_common.h \
	gb_apu/boost/config.hpp gb_apu/boost/cstdint.hpp \
	gb_apu/boost/static_assert.hpp gb_apu/Blip_Synth.h gb_apu/Multi_Buffer.h
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
	rm -f peanut-sdl peanut_sdl.o audio.o Basic_Gb_Apu.o \
		gb_apu/Blip_Buffer.o gb_apu/Gb_Apu.o gb_apu/Gb_Oscs.o \
		gb_apu/Multi_Buffer.o

.SUFFIXES: .c .cpp .o
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@
