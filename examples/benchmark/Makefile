.POSIX:
CC		= cc
OPT		= -s -Ofast
CFLAGS		= $(OPT) -std=c99 -Wall -Wextra

all: peanut-benchmark peanut-benchmark-prof peanut-benchmark-rom
peanut-benchmark: ../../peanut_gb.h peanut_benchmark.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) 
peanut-benchmark-rom: ../../peanut_gb.h peanut_benchmark_rom.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) 
peanut-benchmark-prof: ../../peanut_gb.h peanut_benchmark_prof.c prof.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS) 

clean:
	rm -f peanut-benchmark peanut-benchmark-prof peanut-benchmark-rom
