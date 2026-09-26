CC ?= 
CFLAGS = -Ofast -s

.PHONY: all clean x11 fb

LDLIBS_X11 = -lX11
LDLIBS_FB = -lm -lpthread

all: x11

x11: peanut_minifb.c X11MiniFB.c
	$(CC) $(CFLAGS) -o peanut-minifb $^ $(LDLIBS_X11)

fb: peanut_minifb.c fbMiniFB.c
	$(CC) $(CFLAGS) -o peanut-minifb $^ $(LDLIBS_FB)

clean:
	$(RM) peanut-minifb
