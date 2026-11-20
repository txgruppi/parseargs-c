CFLAGS = -std=c99 -Wall -Wextra -pedantic -g -fsanitize=address,undefined

.PHONY: all test header clean

all: header test

test: build/test
	./build/test

build/test: test/test.c src/parseargs.c src/parseargs.h
	mkdir -p build
	$(CC) $(CFLAGS) -Isrc test/test.c src/parseargs.c -o $@

header:
	$(CC) -std=c99 -Wall -Wextra -pedantic -fsyntax-only -x c src/parseargs.h
	$(CXX) -Wall -Wextra -fsyntax-only -x c++ src/parseargs.h

clean:
	rm -rf build