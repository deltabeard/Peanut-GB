target = libgb_apu.a
sources = $(wildcard *.cpp)
CXXFLAGS = -ggdb

.PHONY: all
all: $(target)

$(target): Makefile $(sources:.cpp=.o)
	ar rcs $(target) $(sources:.cpp=.o)

.PHONY: clean
clean:
	rm -f $(target) *.o


