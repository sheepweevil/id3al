// Implementation of ouptut functions 
// Copyright 2015 David Gloe.

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unicode/ustdio.h"
#include "unicode/ustring.h"
#include "id3v2.h"

#define TITLE_WIDTH 24

static int print_enc(const char *str, int len, enum id3v2_encoding enc);
static size_t strlen_enc(const char *str, enum id3v2_encoding enc);

//static void print_AENC_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_APIC_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_ASPI_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_COMM_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_COMR_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_ENCR_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_EQU2_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_ETCO_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_GEOB_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_GRID_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_LINK_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_MCDI_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_MLLT_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_OWNE_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_PRIV_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_PCNT_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_POPM_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_POSS_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_RBUF_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_RVA2_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_RVRB_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_SEEK_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_SIGN_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_SYLT_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_SYTC_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
static void print_UFID_frame(struct id3v2_frame_header *fheader,
        uint8_t *fdata, uint32_t fdatalen);
//static void print_USER_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
//static void print_USLT_frame(struct id3v2_frame_header *fheader,
//        uint8_t *fdata, uint32_t fdatalen);
static void print_text_frame(struct id3v2_frame_header *fheader,
        uint8_t *fdata, uint32_t fdatalen, int verbosity);
static void print_TXXX_frame(struct id3v2_frame_header *fheader,
        uint8_t *fdata, uint32_t fdatalen, int verbosity);
static void print_url_frame(struct id3v2_frame_header *fheader,
        uint8_t *fdata, uint32_t fdatalen, int verbosity);
static void print_WXXX_frame(struct id3v2_frame_header *fheader,
        uint8_t *fdata, uint32_t fdatalen, int verbosity);

// Print an id3v2 header
void print_id3v2_header(struct id3v2_header *header, int verbosity) {
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

// Print an id3v2 extended header
void print_id3v2_extended_header(struct id3v2_extended_header *eheader,
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
            i++;
            printf("%*s: %s\n", TITLE_WIDTH, "Tag Size Restriction",
                    tag_size_restrict_str(get_tag_size_restriction(
                            eheader->flag_data[i])));
            printf("%*s: %s\n", TITLE_WIDTH, "Text Restriction",
                    text_enc_restrict_str(get_text_encoding_restriction(
                            eheader->flag_data[i])));
            printf("%*s: %s\n", TITLE_WIDTH, "Text Size Restriction",
                    text_size_restrict_str(get_text_size_restriction(
                            eheader->flag_data[i])));
            printf("%*s: %s\n", TITLE_WIDTH, "Image Restriction",
                    img_enc_restrict_str(get_image_encoding_restriction(
                            eheader->flag_data[i])));
            printf("%*s: %s\n", TITLE_WIDTH, "Image Size Restriction",
                    img_size_restrict_str(get_image_size_restriction(
                            eheader->flag_data[i])));
        }
    }
}

// Print an id3v2 frame header
void print_id3v2_frame_header(struct id3v2_frame_header *fheader,
        uint8_t group_id, uint32_t frame_data_len, int verbosity) {
    assert(fheader);

    if (verbosity > 0) {
        printf("%*s: %.*s\n", TITLE_WIDTH, "Frame ID", ID3V2_FRAME_ID_SIZE,
                fheader->id);
        printf("%*s: %"PRIu32" bytes\n", TITLE_WIDTH, "Frame Size",
                fheader->size);

        if (fheader->format_flags & ID3V2_FRAME_HEADER_GROUPING_BIT) {
            printf("%*s: %"PRIu8"\n", TITLE_WIDTH, "Grouping Identifier",
                    group_id);
        }
        if (fheader->format_flags & ID3V2_FRAME_HEADER_DATA_LENGTH_BIT) {
            printf("%*s: %"PRIu32"\n", TITLE_WIDTH, "Data Length",
                    frame_data_len);
        }

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

// Print the string with the given encoding. len should be -1 for NULL
// terminated strings and the string length in bytes otherwise.
// Returns the number of bytes printed on success, -1 otherwise.
static int print_enc(const char *str, int len, enum id3v2_encoding enc) {
    UChar *text;
    UErrorCode uerr;
    int ret;
    int32_t textlen;

    assert(str);

    switch (enc) {
        case ID3V2_ENCODING_UTF_16:
        case ID3V2_ENCODING_UTF_16BE:
            if (len == -1) {
                text = (UChar *)str;
            } else {
                text = malloc(len + sizeof(UChar));
                if (text == NULL) {
                    debug("malloc %d failed: %m", len + 2);
                    return -1;
                }
                memcpy(text, str, len);
                text[len / sizeof(UChar)] = 0;
            }
            ret = u_printf("%S", text);
            if (len != -1) {
                free(text);
            }
            break;
        case ID3V2_ENCODING_UTF_8:
            // Must convert to UTF-16 before printing
            if (len == -1) {
                textlen = strlen(str) * 2 + 2;
            } else {
                text = malloc(len * 2 + 2);
                if (text == NULL) {
                    debug("malloc %d failed: %m", len * 2 + 2);
                    return -1;
                }
            }
            text = malloc(textlen);
            if (text == NULL) {
                debug("malloc %"PRId32" failed: %m", textlen);
                return -1;
            }
            u_strFromUTF8(text, textlen, NULL, str, len, &uerr);
            if (U_FAILURE(uerr)) {
                debug("Conversion from UTF-8 failed: %s", u_errorName(uerr));
                free(text);
                return -1;
            }
            ret = u_printf("%S", text);
            free(text);
            break;
        default:
            if (len == -1) {
                ret = printf("%s", str);
            } else {
                ret = printf("%.*s", len, str);
            }
            break;
    }
    return ret;
}

// Get the length of a terminated encoded string in bytes,
// including the terminator.
static size_t strlen_enc(const char *str, enum id3v2_encoding enc) {
    switch (enc) {
        case ID3V2_ENCODING_UTF_16:
        case ID3V2_ENCODING_UTF_16BE:
            return u_strlen((UChar *)str) * sizeof(UChar) + sizeof(UChar);
        default:
            break;
    }
    return strlen(str) + 1;
}

// Print a UFID frame
static void print_UFID_frame(struct id3v2_frame_header *fheader,
        uint8_t *fdata, uint32_t fdatalen) {
    struct id3v2_frame_UFID frame;
    size_t i;
    const char *title;

    parse_UFID_frame(fdata, &frame);
    title = frame_title(fheader);
    printf("%*s: %s - %s\n", TITLE_WIDTH, title, "Owner", frame.owner);
    printf("%*s: ", TITLE_WIDTH, title);
    for (i = 0; i < fdatalen - strlen(frame.owner) - 1; i++) {
        printf("%"PRIx8" ", frame.id[i]);
    }
    printf("\n");
}

// Print any text frame except TXXX
static void print_text_frame(struct id3v2_frame_header *fheader,
        uint8_t *fdata, uint32_t fdatalen, int verbosity) {
    const char *title = frame_title(fheader);
    struct id3v2_frame_text frame;

    parse_text_frame(fdata, &frame);
    if (verbosity > 0) {
        printf("%*s: %s - %s\n", TITLE_WIDTH, title, "Encoding",
                encoding_str(frame.encoding));
    }
    printf("%*s: ", TITLE_WIDTH, title);
    print_enc(frame.text, fdatalen - 1, frame.encoding);
    printf("\n");
}

// Print a TXXX frame
static void print_TXXX_frame(struct id3v2_frame_header *fheader,
        uint8_t *fdata, uint32_t fdatalen, int verbosity) {
    const char *title = frame_title(fheader);
    struct id3v2_frame_TXXX frame;

    parse_TXXX_frame(fdata, &frame);
    if (verbosity > 0) {
        printf("%*s: %s - %s\n", TITLE_WIDTH, title, "Encoding",
                encoding_str(frame.encoding));
    }
    printf("%*s: %s - ", TITLE_WIDTH, title, "Description");
    print_enc(frame.description, -1, frame.encoding);
    printf("\n");
    printf("%*s: %s - ", TITLE_WIDTH, title, "Value");
    print_enc(frame.value,
            fdatalen - strlen_enc(frame.description, frame.encoding) - 1,
            frame.encoding);
    printf("\n");
}

// Print any URL frame except WXXX
static void print_url_frame(struct id3v2_frame_header *fheader,
        uint8_t *fdata, uint32_t fdatalen, int verbosity) {
    printf("%*s: %.*s\n", TITLE_WIDTH, frame_title(fheader), fdatalen,
            (char *)fdata);
}

// Print a WXXX frame
static void print_WXXX_frame(struct id3v2_frame_header *fheader,
        uint8_t *fdata, uint32_t fdatalen, int verbosity) {
    const char *title = frame_title(fheader);
    struct id3v2_frame_WXXX frame;

    parse_WXXX_frame(fdata, &frame);
    if (verbosity > 0) {
        printf("%*s: %s - %s\n", TITLE_WIDTH, title, "Encoding",
                encoding_str(frame.encoding));
    }
    printf("%*s: %s - ", TITLE_WIDTH, title, "Description");
    print_enc(frame.description, -1, frame.encoding);
    printf("\n");
    printf("%*s: %s - %.*s\n", TITLE_WIDTH, title, "URL",
            (int)(fdatalen -
                strlen_enc(frame.description, frame.encoding) - 1),
            frame.url);
}

// Print an id3v2 frame
void print_id3v2_frame(struct id3v2_frame_header *header,
        uint8_t *fdata, uint32_t fdatalen, int verbosity) {
    if (!strncmp(header->id, ID3V2_FRAME_ID_UFID, ID3V2_FRAME_ID_SIZE)) {
        print_UFID_frame(header, fdata, fdatalen);
    } else if (!strncmp(header->id, ID3V2_FRAME_ID_TXXX,
                ID3V2_FRAME_ID_SIZE)) {
        print_TXXX_frame(header, fdata, fdatalen, verbosity);
    } else if (header->id[0] == 'T') {
        print_text_frame(header, fdata, fdatalen, verbosity);
    } else if (!strncmp(header->id, ID3V2_FRAME_ID_WXXX,
                ID3V2_FRAME_ID_SIZE)) {
        print_WXXX_frame(header, fdata, fdatalen, verbosity);
    } else if (header->id[0] == 'W') {
        print_url_frame(header, fdata, fdatalen, verbosity);
    } else {
        printf("Support for frame %.*s not implemented yet\n",
                ID3V2_FRAME_ID_SIZE, header->id);
    }
    if (verbosity > 0) {
        printf("\n");
    }
}
