SRCS = mergesort.c test.c
HDRS = mergesort.h

CFLAGS = $(shell pkg-config --cflags glib-2.0)
LIBS = $(shell pkg-config --libs glib-2.0)

test: $(SRCS) $(HDRS)
	gcc -std=c99 -Wall -O2 -o test $(CFLAGS) $(SRCS) $(LIBS)

clean:
	rm -rf test
