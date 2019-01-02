CFLAGS = -Ofast -s
LDLIBS = -lX11

all: peanut-minifb
peanut-minifb: peanut_minifb.c X11MiniFB.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

clean:
	$(RM) peanut-minifb
