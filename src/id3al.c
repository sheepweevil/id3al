// Implementation of the id3al command
// Copyright 2015 David Gloe

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2.h"

static void print_usage(const char *name, FILE *fp);
static void parse_args(int argc, char * const argv[], int *verbosity);

// Print usage information to stdout
static void print_usage(const char *name, FILE *fp) {
    fprintf(fp, "Usage: %s [-h] [-v] FILE...\n"
            "    -h, --help:    Print this message\n"
            "    -v, --verbose: Print more information\n"
            "    FILE:          One or more audio files to read\n", name);
    return;
}

// Parse arguments
static void parse_args(int argc, char * const argv[], int *verbosity) {
    int opt;
    struct option longopts[] = {
        {"help", no_argument, NULL, 'h'},
        {"verbose", no_argument, NULL, 'v'},
        {NULL, 0, NULL, 0}
    };

    assert(verbosity);

    *verbosity = 0;
    while ((opt = getopt_long(argc, argv, "hv", longopts, NULL)) != -1) {
        switch (opt) {
            case 'v':
                (*verbosity)++;
                break;
            case 'h':
                print_usage(argv[0], stdout);
                exit(0);
                break;
            default:
                print_usage(argv[0], stderr);
                exit(1);
                break;
        }
    }
    if (optind >= argc) {
        print_usage(argv[0], stderr);
        exit(1);
    }
    return;
}

// Main function
int main(int argc, char * const argv[]) {
    struct id3v2_header header;
    struct id3v2_frame_header fheader;
    int verbosity, fd, i;

    parse_args(argc, argv, &verbosity);

    for (i = optind; i < argc; i++) {
        fd = open(argv[i], O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "Couldn't open %s: %m\n", argv[i]);
            return 1;
        }

        if (get_id3v2_tag(fd, &header)) {
            return 1;
        }

        print_id3v2_header(&header, verbosity);
        if (header.extheader_present) {
            print_id3v2_extended_header(&header.extheader, verbosity);
        }

        while (get_id3v2_frame(&header, &fheader)) {
            print_id3v2_frame_header(&fheader, verbosity);
            print_id3v2_frame(&fheader, verbosity);
            free(fheader.data);
        }
    }
    free(header.frame_data);
    return 0;
}
