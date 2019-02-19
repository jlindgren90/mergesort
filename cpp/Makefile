HDRS = mergesort.h timsort.h

CFLAGS = $(shell pkg-config --cflags glib-2.0)
LIBS = $(shell pkg-config --libs glib-2.0)

all: test bench

test: test.cc $(HDRS)
	g++ -std=c++11 -g -Wall -O2 -o test $(CFLAGS) test.cc $(LIBS)

bench: bench.cc $(HDRS)
	g++ -std=c++14 -g -Wall -O2 -o bench bench.cc

clean:
	rm -rf test bench
