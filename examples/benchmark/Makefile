.POSIX:
CC		:= cc
OPT		:= -s -Ofast
CFLAGS		= $(OPT) -std=c99 -Wall -Wextra
OUTSWITCH	:= -o

ifeq ($(CC),cl)
	OUTSWITCH = /Fe
	CFLAGS := /O2 /W2 /utf-8
	RM := del
	EXT := .exe
endif

all: peanut-benchmark
peanut-benchmark: peanut-benchmark.c ../../peanut_gb.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(OUTSWITCH)$@ $< $(LDLIBS)

peanut-benchmark.S: peanut-benchmark.c ../../peanut_gb.h
	$(CC) -S $(CFLAGS) $(LDFLAGS) $(OUTSWITCH)$@ $< $(LDLIBS)

clean:
	$(RM) peanut-benchmark$(EXT)
