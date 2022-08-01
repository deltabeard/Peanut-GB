.POSIX:
CC		:= cc
OPT		:= -s -Ofast
CFLAGS		= $(OPT) -std=c99 -Wall -Wextra
OUTSWITCH	:= -o

ifeq ($(CC),cl)
	OUTSWITCH = /Fe
	CFLAGS := /O2 /W2 /utf-8
endif

all: peanut-benchmark peanut-benchmark-rom
peanut-benchmark: peanut_benchmark.c ../../peanut_gb.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(OUTSWITCH)$@ $< $(LDLIBS)
peanut-benchmark-rom: peanut_benchmark_rom.c ../../peanut_gb.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(OUTSWITCH)$@ $< $(LDLIBS)

clean:
	rm -f peanut-benchmark peanut-benchmark-rom
