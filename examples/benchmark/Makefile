.POSIX:
CC		= cc
OPT		= -s -Ofast
CFLAGS	= $(OPT) -Wall

all: peanut-benchmark peanut-benchmark-prof
peanut-benchmark: ../../peanut_gb.h peanut_benchmark.c
	$(CC) peanut_benchmark.c -o peanut-benchmark $(CFLAGS) $(LDLIBS)
peanut-benchmark-prof: ../../peanut_gb.h peanut_benchmark_prof.c prof.h
	$(CC) peanut_benchmark_prof.c -o peanut-benchmark-prof $(CFLAGS) $(LDLIBS)
clean:
	rm -f peanut-benchmark peanut-benchmark-prof
