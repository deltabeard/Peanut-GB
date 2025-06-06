OPT=-g2 -O2

override CFLAGS += $(OPT) -Wall -Wextra -DMINIGB_APU_AUDIO_FORMAT_S16SYS

all: test test_so
test: test.o ../examples/sdl2/minigb_apu/minigb_apu.o
	$(CC) $^ -o $@ $(CFLAGS)

test_so: test.c peanut_gb.o ../examples/sdl2/minigb_apu/minigb_apu.o
	$(CC) $^ -o $@ -DPEANUT_GB_HEADER_ONLY $(CFLAGS)

test_external_rom: test_external_rom.c
	$(CC) $^ -o $@ $(CFLAGS)

peanut_gb.o: ../peanut_gb.h
	cp ../peanut_gb.h ./peanut_gb.c
	$(CC) -c peanut_gb.c -o $@ $(CFLAGS)
	$(CC) -c peanut_gb.c -S -o $@.S $(CFLAGS)
