.POSIX:
CC		= cc
OPT		= -s -Ofast
CFLAGS	= $(OPT) -Wall

all: gameboy_benchmark gameboy_benchmark_prof
gameboy_benchmark: ../../gameboy.h gameboy_benchmark.c
	$(CC) gameboy_benchmark.c -o gameboy_benchmark $(CFLAGS) $(LDLIBS)
gameboy_benchmark_prof: ../../gameboy.h gameboy_benchmark_prof.c prof.h
	$(CC) gameboy_benchmark_prof.c -o gameboy_benchmark_prof $(CFLAGS) $(LDLIBS)
clean:
	rm -f gameboy_benchmark gameboy_benchmark_prof
