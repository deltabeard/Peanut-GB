
SDL2_CFLAGS := $(shell sdl2-config --cflags)
SDL2_LDLIBS := $(shell sdl2-config --libs)
override CFLAGS += -Iinc $(SDL2_CFLAGS)
override LDLIBS += $(SDL2_LDLIBS)

all: peanutgb-debugger
peanutgb-debugger: src/main.o src/nuklear.o src/overview.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)
