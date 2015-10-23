# Makefile for the id3al project
# Copyright 2015 David Gloe.

CFLAGS=-Wall -Werror -DDEBUG

check: src/tests/id3test
	./src/tests/id3test 
src/tests/id3test: src/tests/id3test.o src/synchronize.o src/decode.o
src/tests/id3test.o: src/id3v2.h
src/decode.o: src/id3v2.h
src/synchronize.o: src/id3v2.h

.PHONY: clean
clean:
	rm -f src/tests/id3test.o src/synchronize.o src/tests/id3test
