# id3al
A command-line ID3 tag viewer for Linux.

## Build

First install packages needed to build. On Ubuntu use

    sudo apt-get install build-essential libicu-dev

Then build with

    make

Optionally, run tests with

    make check

## Run

To use, execute

    ./src/id3al <MP3 file>

Add `-v` arguments to produce more detailed output.
