CC		:= cc
OPT		:= -s -O2
CFLAGS	:= $(OPT) -std=c99 -Wall -Wextra

override peanut-benchmark.o: CFLAGS += -DPEANUT_GB_HEADER_ONLY

all: peanut-benchmark-obj
peanut-benchmark-obj: peanut-benchmark.o peanut_gb.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS $(LDLIBS)

peanut_gb.c: ../../peanut_gb.h
	cp $< $@

