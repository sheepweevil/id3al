# Makefile for the id3al project
# Copyright 2015 David Gloe.

CFLAGS=-Wall -Werror -DDEBUG

all: src/id3al
src/id3al: src/id3al.o src/synchronize.o src/decode.o src/verify.o
src/id3al.o: src/id3v2.h

check: src/tests/id3test
	./src/tests/id3test 
src/tests/id3test: src/tests/id3test.o src/synchronize.o src/decode.o src/verify.o
src/tests/id3test.o: src/id3v2.h
src/decode.o: src/id3v2.h
src/synchronize.o: src/id3v2.h
src/verify.o: src/id3v2.h

.PHONY: clean
clean:
	rm -f src/tests/*.o src/*.o src/tests/id3test src/id3al
