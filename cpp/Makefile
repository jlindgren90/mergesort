SRCS = test.cc
HDRS = mergesort.h timsort.h

CFLAGS = $(shell pkg-config --cflags glib-2.0)
LIBS = $(shell pkg-config --libs glib-2.0)

test: $(SRCS) $(HDRS)
	g++ -std=c++11 -g -Wall -O2 -o test $(CFLAGS) $(SRCS) $(LIBS)

clean:
	rm -rf test
