# Makefile for the id3al project
# Copyright 2015 David Gloe.

CFLAGS=-Wall -Werror -DDEBUG -g `pkg-config --cflags icu-uc icu-io zlib`
LDLIBS=`pkg-config --libs icu-uc icu-io zlib`

all: src/id3al

check: src/tests/id3test
	./src/tests/id3test 

src/id3al: src/convert.o src/decode.o src/output.o src/synchronize.o src/verify.o
src/tests/id3test: src/convert.o src/decode.o src/synchronize.o src/verify.o

src/tests/id3test.o: src/id3v2.h
src/id3al.o: src/id3v2.h
src/convert.o: src/id3v2.h
src/decode.o: src/id3v2.h
src/output.o: src/id3v2.h
src/synchronize.o: src/id3v2.h
src/verify.o: src/id3v2.h

.PHONY: clean
clean:
	rm -f src/tests/*.o src/*.o src/tests/id3test src/id3al
