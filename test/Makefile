OPT=-g2 -O2
override CFLAGS=$(OPT) -Wall -Werror

all: test
	$(CC) test.c -o test $(CFLAGS)
	./test
