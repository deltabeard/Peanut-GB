target = peanut_sdl
objects = peanut_sdl.o audio.o Basic_Gb_Apu.o gb_apu/libgb_apu.a
CFLAGS	:= $(shell sdl2-config --cflags) -std=c99 -s -Os -W -Wall -Wextra
CXXFLAGS= $(shell sdl2-config --cflags) -s -Os -Wall
LDFLAGS	:= $(shell sdl2-config --libs) -lm

.PHONY: all
all: $(target)

$(target): Makefile $(objects)
	$(CXX) $(CFLAGS) $(objects) $(LDFLAGS) -o $(target)

.PHONY: clean
clean:
	rm -Rf $(target) *.o
	$(MAKE) -C gb_apu clean

gb_apu/libgb_apu.a: Makefile
	$(MAKE) -C gb_apu

