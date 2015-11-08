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

#define TITLE_WIDTH 24

static void print_usage(const char *name, FILE *fp);
static void parse_args(int argc, char * const argv[], int *verbosity);
static char *boolstr(int b);
static void print_id3v2_header(struct id3v2_header *header, int verbosity);
static void print_id3v2_extended_header(struct id3v2_extended_header *eheader,
        int verbosity);
static void print_id3v2_frame_header(struct id3v2_frame_header *fheader,
        int verbosity);

static void print_usage(const char *name, FILE *fp) {
    fprintf(fp, "Usage: %s [-h] [-v] FILE...\n"
            "    -h, --help:    Print this message\n"
            "    -v, --verbose: Print more information\n"
            "    FILE:          One or more audio files to read\n", name);
    return;
}

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

static char *boolstr(int b) {
    if (b) {
        return "True";
    }
    return "False";
}

static void print_id3v2_header(struct id3v2_header *header, int verbosity) {
    assert(header);

    if (verbosity > 0) {
        printf("%*s: 2.%"PRIu8".%"PRIu8"\n", TITLE_WIDTH, "ID3 Version",
                header->version, header->revision);
        printf("%*s: %"PRIu32" bytes\n", TITLE_WIDTH, "Tag Size",
                from_synchsafe(header->tag_size));
    }

    if (verbosity > 1) {
        printf("%*s: %s\n", TITLE_WIDTH, "Unsynchronization",
                boolstr(header->flags & ID3V2_HEADER_UNSYNCHRONIZATION_BIT));
        printf("%*s: %s\n", TITLE_WIDTH, "Extended Header",
                boolstr(header->flags & ID3V2_HEADER_EXTENDED_HEADER_BIT));
        printf("%*s: %s\n", TITLE_WIDTH, "Experimental",
                boolstr(header->flags & ID3V2_HEADER_EXPERIMENTAL_BIT));
        printf("%*s: %s\n", TITLE_WIDTH, "Footer",
                boolstr(header->flags & ID3V2_HEADER_FOOTER_BIT));
    }

    if (verbosity > 0) {
        printf("\n");
    }
    return;
}

static void print_id3v2_extended_header(struct id3v2_extended_header *eheader,
        int verbosity) {
    assert(eheader);

    size_t i = 0;
    uint32_t crc;

    if (verbosity > 0) {
        printf("%*s: %"PRIu32" bytes\n", TITLE_WIDTH, "Extended Header Size",
                from_synchsafe(eheader->size));
    }

    if (verbosity > 1) {
        printf("%*s: %s\n", TITLE_WIDTH, "Tag is an Update",
                boolstr(eheader->flags & ID3V2_EXTENDED_HEADER_UPDATE_BIT));
        if (eheader->flags & ID3V2_EXTENDED_HEADER_UPDATE_BIT) {
            i++;
        }
        if (eheader->flags & ID3V2_EXTENDED_HEADER_CRC_BIT) {
            i++;
            crc = eheader->flag_data[i] << 28;
            i++;
            crc += from_synchsafe(*(uint32_t *)(eheader->flag_data + i));
            printf("%*s: %"PRIx32"\n", TITLE_WIDTH, "CRC-32", crc);
        }
        if (eheader->flags & ID3V2_EXTENDED_HEADER_TAG_RESTRICTIONS_BIT) {
            printf("%*s: ", TITLE_WIDTH, "Tag Size Restriction");
            i++;
            switch (get_tag_size_restriction(eheader->flag_data[i])) {
                case ID3V2_RESTRICTION_TAG_SIZE_1MB:
                    printf("No more than 128 frames and 1MB tag size\n");
                    break;
                case ID3V2_RESTRICTION_TAG_SIZE_128KB:
                    printf("No more than 64 frames and 128KB tag size\n");
                    break;
                case ID3V2_RESTRICTION_TAG_SIZE_40KB:
                    printf("No more than 32 frames and 40KB tag size\n");
                    break;
                case ID3V2_RESTRICTION_TAG_SIZE_4KB:
                    printf("No more than 32 frames and 4KB tag size\n");
                    break;
                default:
                    break;
            }
            printf("%*s: ", TITLE_WIDTH, "Text Restriction");
            switch (get_text_encoding_restriction(eheader->flag_data[i])) {
                case ID3V2_RESTRICTION_TEXT_ENCODING_NONE:
                    printf("No text encoding restrictions\n");
                    break;
                case ID3V2_RESTRICTION_TEXT_ENCODING_BYTE:
                    printf("Text encoded with ISO-8859-1 or UTF-8\n");
                    break;
                default:
                    break;
            }
            printf("%*s: ", TITLE_WIDTH, "Text Size Restriction");
            switch (get_text_size_restriction(eheader->flag_data[i])) {
                case ID3V2_RESTRICTION_TEXT_SIZE_NONE:
                    printf("No text size restrictions\n");
                    break;
                case ID3V2_RESTRICTION_TEXT_SIZE_1024:
                    printf("No string is longer than 1024 characters\n");
                    break;
                case ID3V2_RESTRICTION_TEXT_SIZE_128:
                    printf("No string is longer than 128 characters\n");
                    break;
                case ID3V2_RESTRICTION_TEXT_SIZE_30:
                    printf("No string is longer than 30 characters\n");
                    break;
                default:
                    break;
            }
            printf("%*s: ", TITLE_WIDTH, "Image Restriction");
            switch (get_image_encoding_restriction(eheader->flag_data[i])) {
                case ID3V2_RESTRICTION_IMAGE_ENCODING_NONE:
                    printf("No image encoding restrictions\n");
                    break;
                case ID3V2_RESTRICTION_IMAGE_ENCODING_COMPRESSED:
                    printf("Images encoded with PNG or JPEG\n");
                    break;
                default:
                    break;
            }
            printf("%*s: ", TITLE_WIDTH, "Image Size Restriction");
            switch(get_image_size_restriction(eheader->flag_data[i])) {
                case ID3V2_RESTRICTION_IMAGE_SIZE_NONE:
                    printf("No image size restrictions\n");
                    break;
                case ID3V2_RESTRICTION_IMAGE_SIZE_256:
                    printf("All images are 256x256 pixels or smaller\n");
                    break;
                case ID3V2_RESTRICTION_IMAGE_SIZE_64:
                    printf("All images are 64x64 pixels or smaller\n");
                    break;
                case ID3V2_RESTRICTION_IMAGE_SIZE_64_STRICT:
                    printf("All images are exactly 64x64 pixels\n");
                    break;
                default:
                    break;
            }
        }
    }
}

static void print_id3v2_frame_header(struct id3v2_frame_header *fheader,
        int verbosity) {
    if (verbosity > 0) {
        printf("%*s: %.*s\n", TITLE_WIDTH, "Frame ID", ID3V2_FRAME_ID_SIZE,
                fheader->id);
        printf("%*s: %"PRIu32" bytes\n", TITLE_WIDTH, "Frame Size",
                fheader->size);
    }

    if (verbosity > 1) {
        printf("%*s: %s\n", TITLE_WIDTH, "Tag Alter Discard",
                boolstr(fheader->status_flags &
                    ID3V2_FRAME_HEADER_TAG_ALTER_BIT));
        printf("%*s: %s\n", TITLE_WIDTH, "File Alter Discard",
                boolstr(fheader->status_flags &
                    ID3V2_FRAME_HEADER_FILE_ALTER_BIT));
        printf("%*s: %s\n", TITLE_WIDTH, "Read Only",
                boolstr(fheader->status_flags &
                    ID3V2_FRAME_HEADER_READ_ONLY_BIT));

        printf("%*s: %s\n", TITLE_WIDTH, "Group Information",
                boolstr(fheader->format_flags &
                    ID3V2_FRAME_HEADER_GROUPING_BIT));
        printf("%*s: %s\n", TITLE_WIDTH, "Compression",
                boolstr(fheader->format_flags &
                    ID3V2_FRAME_HEADER_COMPRESSION_BIT));
        printf("%*s: %s\n", TITLE_WIDTH, "Encryption",
                boolstr(fheader->format_flags &
                    ID3V2_FRAME_HEADER_ENCRYPTION_BIT));
        printf("%*s: %s\n", TITLE_WIDTH, "Unsynchronization",
                boolstr(fheader->format_flags &
                    ID3V2_FRAME_HEADER_UNSYNCHRONIZATION_BIT));
        printf("%*s: %s\n", TITLE_WIDTH, "Data Length Indicator",
                boolstr(fheader->format_flags &
                    ID3V2_FRAME_HEADER_DATA_LENGTH_BIT));
    }
}

void print_id3v2_frames(struct id3v2_header *header,
        struct id3v2_extended_header *eheader, uint8_t *frame_data,
        size_t frame_data_len, int verbosity) {
    struct id3v2_frame_header *fheader;
    uint8_t *fdata;
    size_t i = 0;

    while (!get_id3v2_frame(header, frame_data, frame_data_len, &i, &fheader,
                &fdata)) {
        print_id3v2_frame_header(fheader, verbosity);
    }
}

int main(int argc, char * const argv[]) {
    struct id3v2_header header;
    struct id3v2_footer footer;
    struct id3v2_extended_header extheader;
    uint8_t *frame_data;
    size_t frame_data_len;
    int verbosity, i, fd;

    parse_args(argc, argv, &verbosity);

    for (i = optind; i < argc; i++) {
        fd = open(argv[i], O_RDONLY);
        if (fd == -1) {
            fprintf(stderr, "Couldn't open %s: %m\n", argv[i]);
            return 1;
        }

        if (get_id3v2_tag(fd, &header, &extheader, &frame_data,
                    &frame_data_len, &footer)) {
            return 1;
        }

        print_id3v2_header(&header, verbosity);
        if (header.flags & ID3V2_HEADER_EXTENDED_HEADER_BIT) {
            print_id3v2_extended_header(&extheader, verbosity);
        }

        print_id3v2_frames(&header, &extheader, frame_data, frame_data_len,
                verbosity);
    }
    return 0;
}
