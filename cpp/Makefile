HDRS = mergesort.h timsort.h

all: test bench

test: test.cc $(HDRS)
	g++ -std=c++11 -g -Wall -O2 -o test test.cc

bench: bench.cc $(HDRS)
	g++ -std=c++14 -g -Wall -O2 -o bench bench.cc

clean:
	rm -rf test bench
