OPT=-g2 -O2 -DPEANUT_GB_HIGH_LCD_ACCURACY=0

override CFLAGS += $(OPT) -Wall -Wextra

all: test test_so
test: test.o
	$(CC) $< -o $@ $(CFLAGS)

test_so: test.c peanut_gb.o
	$(CC) $^ -o $@ -DPEANUT_GB_HEADER_ONLY $(CFLAGS)

test_external_rom: test_external_rom.c
	$(CC) $^ -o $@ $(CFLAGS)

peanut_gb.o: ../peanut_gb.h
	cp ../peanut_gb.h ./peanut_gb.c
	$(CC) -c peanut_gb.c -o $@ $(CFLAGS)
	$(CC) -c peanut_gb.c -S -o $@.S $(CFLAGS)
