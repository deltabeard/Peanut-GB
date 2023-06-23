.POSIX:
CC		:= cc
OPT		:= -g2 -O2
CFLAGS		= $(OPT) -std=c99 -Wall -Wextra -DPEANUT_GB_12_COLOUR=1
CP		:= cp

peanut-benchmark-sep.o: override CFLAGS += -DPEANUT_GB_HEADER_ONLY

override CFLAGS += -DENABLE_SOUND=0 -DENABLE_LCD=1

all: peanut-benchmark peanut-benchmark-sep
peanut-benchmark: peanut-benchmark.c ../../peanut_gb.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o$@ $< $(LDLIBS)

# Separate objects linked to a single executable.
peanut-benchmark-sep: peanut-benchmark-sep.o peanut_gb.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o$@ $^ $(LDLIBS)

peanut-benchmark-sep.o: peanut-benchmark.c
	$(CC) -c $(CFLAGS) -o$@ $<

peanut_gb.o: ../../peanut_gb.h
	$(CP) ../../peanut_gb.h peanut_gb.c
	$(CC) -c $(CFLAGS) -o$@ peanut_gb.c

peanut-benchmark.S: peanut-benchmark.c ../../peanut_gb.h
	$(CC) -S $(CFLAGS) $(LDFLAGS) -o$@ $< $(LDLIBS)

clean:
	$(RM) peanut-benchmark$(EXT)
