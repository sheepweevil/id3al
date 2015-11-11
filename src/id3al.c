// Implementation of the id3al command
// Copyright 2015 David Gloe

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unicode/ustdio.h"
#include "unicode/ustring.h"
#include "id3v2.h"

#define TITLE_WIDTH 24
#define SUBTITLE_WIDTH 10

static void print_usage(const char *name, FILE *fp);
static void parse_args(int argc, char * const argv[], int *verbosity);
static char *boolstr(int b);
static void print_id3v2_header(struct id3v2_header *header, int verbosity);
static void print_id3v2_extended_header(struct id3v2_extended_header *eheader,
        int verbosity);
static void print_id3v2_frame_header(struct id3v2_frame_header *fheader,
        int verbosity);
static void print_id3v2_frame(struct id3v2_frame_header *header,
        uint8_t *fdata, int verbosity);
static const char *frame_title(struct id3v2_frame_header *fheader);

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
            printf("%*s: 0x%"PRIx32"\n", TITLE_WIDTH, "CRC-32", crc);
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

static const char *frame_title(struct id3v2_frame_header *fheader) {
    static const char *frames[] = {
        ID3V2_FRAME_ID_AENC, ID3V2_FRAME_ID_APIC, ID3V2_FRAME_ID_ASPI,
        ID3V2_FRAME_ID_COMM, ID3V2_FRAME_ID_COMR, ID3V2_FRAME_ID_ENCR,
        ID3V2_FRAME_ID_EQU2, ID3V2_FRAME_ID_ETCO, ID3V2_FRAME_ID_GEOB,
        ID3V2_FRAME_ID_GRID, ID3V2_FRAME_ID_LINK, ID3V2_FRAME_ID_MCDI,
        ID3V2_FRAME_ID_MLLT, ID3V2_FRAME_ID_OWNE, ID3V2_FRAME_ID_PRIV,
        ID3V2_FRAME_ID_PCNT, ID3V2_FRAME_ID_POPM, ID3V2_FRAME_ID_POSS,
        ID3V2_FRAME_ID_RBUF, ID3V2_FRAME_ID_RVA2, ID3V2_FRAME_ID_RVRB,
        ID3V2_FRAME_ID_SEEK, ID3V2_FRAME_ID_SIGN, ID3V2_FRAME_ID_SYLT,
        ID3V2_FRAME_ID_SYTC, ID3V2_FRAME_ID_TALB, ID3V2_FRAME_ID_TBPM,
        ID3V2_FRAME_ID_TCOM, ID3V2_FRAME_ID_TCON, ID3V2_FRAME_ID_TCOP,
        ID3V2_FRAME_ID_TDEN, ID3V2_FRAME_ID_TDLY, ID3V2_FRAME_ID_TDOR,
        ID3V2_FRAME_ID_TDRC, ID3V2_FRAME_ID_TDRL, ID3V2_FRAME_ID_TDTG,
        ID3V2_FRAME_ID_TENC, ID3V2_FRAME_ID_TEXT, ID3V2_FRAME_ID_TFLT,
        ID3V2_FRAME_ID_TIPL, ID3V2_FRAME_ID_TIT1, ID3V2_FRAME_ID_TIT2,
        ID3V2_FRAME_ID_TIT3, ID3V2_FRAME_ID_TKEY, ID3V2_FRAME_ID_TLAN,
        ID3V2_FRAME_ID_TLEN, ID3V2_FRAME_ID_TMCL, ID3V2_FRAME_ID_TMED,
        ID3V2_FRAME_ID_TMOO, ID3V2_FRAME_ID_TOAL, ID3V2_FRAME_ID_TOFN,
        ID3V2_FRAME_ID_TOLY, ID3V2_FRAME_ID_TOPE, ID3V2_FRAME_ID_TOWN,
        ID3V2_FRAME_ID_TPE1, ID3V2_FRAME_ID_TPE2, ID3V2_FRAME_ID_TPE3,
        ID3V2_FRAME_ID_TPE4, ID3V2_FRAME_ID_TPOS, ID3V2_FRAME_ID_TPRO,
        ID3V2_FRAME_ID_TPUB, ID3V2_FRAME_ID_TRCK, ID3V2_FRAME_ID_TRSN,
        ID3V2_FRAME_ID_TRSO, ID3V2_FRAME_ID_TSOA, ID3V2_FRAME_ID_TSOP,
        ID3V2_FRAME_ID_TSOT, ID3V2_FRAME_ID_TSRC, ID3V2_FRAME_ID_TSSE,
        ID3V2_FRAME_ID_TSST, ID3V2_FRAME_ID_TXXX, ID3V2_FRAME_ID_UFID,
        ID3V2_FRAME_ID_USER, ID3V2_FRAME_ID_USLT, ID3V2_FRAME_ID_WCOM,
        ID3V2_FRAME_ID_WCOP, ID3V2_FRAME_ID_WOAF, ID3V2_FRAME_ID_WOAR,
        ID3V2_FRAME_ID_WOAS, ID3V2_FRAME_ID_WORS, ID3V2_FRAME_ID_WPAY,
        ID3V2_FRAME_ID_WPUB, ID3V2_FRAME_ID_WXXX
    };
    static const char *titles[] = {
        "Audio Encryption",
        "Attached Picture",
        "Audio Seek Point Index",
        "Comments",
        "Commercial Info",
        "Encryption Method",
        "Equalization",
        "Event Timing",
        "Encapsulated Object",
        "Group Identification",
        "Linked Info",
        "Music CD",
        "MPEG Lookup Table",
        "Ownership",
        "Private",
        "Play Counter",
        "Popularimeter",
        "Position Sync",
        "Recommended Buffer Size",
        "Relative Volume Adjust",
        "Reverb",
        "Seek",
        "Signature",
        "Synchronized Lyrics",
        "Synchronized Tempo",
        "Album Title",
        "BPM",
        "Composer",
        "Content Type",
        "Copyright",
        "Encoding Time",
        "Playlist Delay",
        "Original Release Time",
        "Recording Time",
        "Release Time",
        "Tagging Time",
        "Encoded By",
        "Lyricist",
        "File Type",
        "Involved People",
        "Content Group",
        "Title",
        "Subtitle",
        "Initial Key",
        "Language",
        "Length",
        "Musician Credits List",
        "Media Type",
        "Mood",
        "Original Album Title",
        "Original Filename",
        "Original Lyricist",
        "Original Artist",
        "File Owner",
        "Lead Performer",
        "Accompaniment",
        "Conductor",
        "Interpreted By",
        "Part of a Set",
        "Produced Notice",
        "Publisher",
        "Track Number",
        "Radio Station Name",
        "Radio Station Owner",
        "Album Sort Order",
        "Performer Sort Order",
        "Title Sort Order",
        "ISRC Code",
        "Encoding Settings",
        "Set Subtitle",
        "Text Info",
        "Unique File ID",
        "Terms of Use",
        "Lyrics",
        "Commercial Webpage",
        "Copyright Webpage",
        "Audio Webpage",
        "Artist Webpage",
        "Audio Source Webpage",
        "Radio Station Webpage",
        "Payment Webpage",
        "Publisher Webpage",
        "Webpage"
    };
    size_t i;

    for (i = 0; i < sizeof(frames) / sizeof(const char *); i++) {
        if (!strncmp(fheader->id, frames[i], ID3V2_FRAME_ID_SIZE)) {
            return titles[i];
        }
    }
    return "Unknown Tag";
}

static void print_UFID_frame(struct id3v2_frame_header *fheader,
        uint8_t *fdata) {
    struct id3v2_frame_UFID frame;
    size_t i;

    parse_UFID_frame(fdata, &frame);
    printf("%*s: %s\n", TITLE_WIDTH, "Unique File ID Owner", frame.owner);
    printf("%*s: ", TITLE_WIDTH, "Unique File ID");
    for (i = 0; i < fheader->size - strlen(frame.owner) - 1; i++) {
        printf("%"PRIx8" ", frame.id[i]);
    }
    printf("\n");
}

static char *encoding_str(enum id3v2_encoding enc) {
    switch (enc) {
        case ID3V2_ENCODING_ISO_8859_1:
            return "ISO 8859-1";
        case ID3V2_ENCODING_UTF_16:
            return "UTF-16 with BOM";
        case ID3V2_ENCODING_UTF_16BE:
            return "UTF-16 without BOM";
        case ID3V2_ENCODING_UTF_8:
            return "UTF-8";
        default:
            break;
    }
    return "Unknown";
}

static void print_text_frame(struct id3v2_frame_header *fheader,
        uint8_t *fdata, int verbosity) {
    const char *title = frame_title(fheader);
    struct id3v2_frame_text frame;
    UChar *text;
    UErrorCode uerr;

    parse_text_frame(fdata, &frame);
    if (verbosity > 0) {
        printf("%*s: %*s: %s\n", TITLE_WIDTH, title,
                SUBTITLE_WIDTH, "Encoding",
                encoding_str(frame.encoding));
    }
    switch (frame.encoding) {
        case ID3V2_ENCODING_UTF_16:
        case ID3V2_ENCODING_UTF_16BE:
            // Add a null terminator before printing
            text = malloc(fheader->size + 1);
            if (text == NULL) {
                debug("malloc %"PRIu32" failed: %m", fheader->size * 2);
                break;
            }
            memcpy(text, frame.text, fheader->size - 1);
            text[fheader->size] = 0;
            text[fheader->size - 1] = 0;
            u_printf("%*s: %S\n", TITLE_WIDTH, title, text);
            free(text);
            break;
        case ID3V2_ENCODING_UTF_8:
            // Must convert to UTF-16 before printing
            text = malloc(fheader->size * 2);
            if (text == NULL) {
                debug("malloc %"PRIu32" failed: %m", fheader->size * 2);
                break;
            }
            u_strFromUTF8(text, fheader->size * 2, NULL, frame.text,
                    fheader->size - 1, &uerr);
            if (U_FAILURE(uerr)) {
                debug("Conversion from UTF-8 failed: %s", u_errorName(uerr));
                free(text);
                break;
            }
            u_printf("%*s: %S\n", TITLE_WIDTH, title, text);
            free(text);
            break;
        default:
            printf("%*s: %.*s\n", TITLE_WIDTH, title, fheader->size - 1,
                    frame.text);
            break;
    }
}

static void print_TXXX_frame(uint8_t *fdata) {

}

static void print_url_frame(uint8_t *fdata) {

}

static void print_WXXX_frame(uint8_t *fdata) {

}

static void print_id3v2_frame(struct id3v2_frame_header *header,
        uint8_t *fdata, int verbosity) {
    size_t index = 0;

    if (header->format_flags & ID3V2_FRAME_HEADER_GROUPING_BIT) {
        if (verbosity > 0) {
            printf("%*s: %"PRIu8"\n", TITLE_WIDTH, "Grouping Identifier",
                    *fdata);
        }
        index++;
    }
    if (header->format_flags & ID3V2_FRAME_HEADER_DATA_LENGTH_BIT) {
        if (verbosity > 0) {
            printf("%*s: %"PRIu32"\n", TITLE_WIDTH, "Data Length",
                    from_synchsafe(byte_swap_32(*(uint32_t *)(fdata + index))));
        }
        index++;
    }

    // TODO: support compression, encryption, unsynchronization
    if (!strncmp(header->id, ID3V2_FRAME_ID_UFID, ID3V2_FRAME_ID_SIZE)) {
        print_UFID_frame(header, fdata);
    } else if (!strncmp(header->id, ID3V2_FRAME_ID_TXXX,
                ID3V2_FRAME_ID_SIZE)) {
        print_TXXX_frame(fdata);
    } else if (header->id[0] == 'T') {
        print_text_frame(header, fdata, verbosity);
    } else if (!strncmp(header->id, ID3V2_FRAME_ID_WXXX,
                ID3V2_FRAME_ID_SIZE)) {
        print_WXXX_frame(fdata);
    } else if (header->id[0] == 'W') {
        print_url_frame(fdata);
    } else {
        printf("Support for frame %.*s not implemented yet\n",
                ID3V2_FRAME_ID_SIZE, header->id);
    }
    if (verbosity > 0) {
        printf("\n");
    }
}

static void print_id3v2_frames(struct id3v2_header *header,
        struct id3v2_extended_header *eheader, uint8_t *frame_data,
        size_t frame_data_len, int verbosity) {
    struct id3v2_frame_header *fheader;
    uint8_t *fdata;
    size_t i = 0;

    while (!get_id3v2_frame(header, frame_data, frame_data_len, &i, &fheader,
                &fdata)) {
        print_id3v2_frame_header(fheader, verbosity);
        print_id3v2_frame(fheader, fdata, verbosity);
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
