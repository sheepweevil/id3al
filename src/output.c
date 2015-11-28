// Implementation of ouptut functions
// Copyright 2015 David Gloe.

#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "unicode/ustdio.h"
#include "unicode/ustring.h"
#include "id3v2.h"

#define TITLE_WIDTH 24

static int print_enc(const char *str, int len, enum id3v2_encoding enc);
static void print_bin(uint8_t *data, size_t len);
static char * write_tmpfile(uint8_t *data, size_t len);

static void print_AENC_frame(struct id3v2_frame_header *fheader,
        int verbosity);
static void print_APIC_frame(struct id3v2_frame_header *fheader,
        int verbosity, int extract);
//static void print_ASPI_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
static void print_COMM_frame(struct id3v2_frame_header *fheader,
        int verbosity);
//static void print_COMR_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_ENCR_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_EQU2_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_ETCO_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_GEOB_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_GRID_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_LINK_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_MCDI_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_MLLT_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_OWNE_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
static void print_PRIV_frame(struct id3v2_frame_header *fheader,
        int verbosity);
//static void print_PCNT_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_POPM_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_POSS_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_RBUF_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_RVA2_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_RVRB_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_SEEK_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_SIGN_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_SYLT_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_SYTC_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
static void print_UFID_frame(struct id3v2_frame_header *fheader,
        int verbosity);
//static void print_USER_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
//static void print_USLT_frame(struct id3v2_frame_header *fheader,
//        int verbosity);
static void print_text_frame(struct id3v2_frame_header *fheader,
        int verbosity);
static void print_TXXX_frame(struct id3v2_frame_header *fheader,
        int verbosity);
static void print_url_frame(struct id3v2_frame_header *fheader,
        int verbosity);
static void print_WXXX_frame(struct id3v2_frame_header *fheader,
        int verbosity);

// Print arbitrary data in sections of four hex digits
static void print_bin(uint8_t *data, size_t len) {
    size_t i;

    for (i = 0; i < len - 1; i += 2) {
        if (i) {
            printf(" ");
        }
        printf("%02"PRIx8"%02"PRIx8, data[i], data[i + 1]);
    }
    if (i == len - 1) {
        if (i) {
            printf(" ");
        }
        printf("%02"PRIx8, data[i]);
    }
}

// Save arbitrary data to a temporary file
// Returns the name of the file, or NULL on failure
// The return value must be freed
static char * write_tmpfile(uint8_t *data, size_t len) {
    char *template;
    int fd;
    size_t written = 0;
    ssize_t count;

    if (asprintf(&template, "id3al-XXXXXX") == -1) {
        debug("asprintf failed");
        return NULL;
    }
    fd = mkstemp(template);
    if (fd == -1) {
        debug("mkstemp failed: %m");
        free(template);
        return NULL;
    }
    while (written < len) {
        count = write(fd, data + written, len - written);
        if (count == -1) {
            if (errno != EINTR && errno != EAGAIN) {
                debug("write failed: %m");
                close(fd);
                unlink(template);
                free(template);
                return NULL;
            }
        } else {
            written += count;
        }
    }
    close(fd);
    return template;
}

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
                boolstr(header->unsynchronization));
        printf("%*s: %s\n", TITLE_WIDTH, "Extended Header",
                boolstr(header->extheader_present));
        printf("%*s: %s\n", TITLE_WIDTH, "Experimental",
                boolstr(header->experimental));
        printf("%*s: %s\n", TITLE_WIDTH, "Footer",
                boolstr(header->footer_present));
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

    if (verbosity > 0) {
        printf("%*s: %"PRIu32" bytes\n", TITLE_WIDTH, "Extended Header Size",
                from_synchsafe(eheader->size));
    }

    if (verbosity > 1) {
        printf("%*s: %s\n", TITLE_WIDTH, "Tag is an Update",
                boolstr(eheader->update));
        if (eheader->crc_present) {
            printf("%*s: 0x%"PRIx32"\n", TITLE_WIDTH, "CRC-32", eheader->crc);
        }
        if (eheader->restrictions) {
            printf("%*s: %s\n", TITLE_WIDTH, "Tag Size Restriction",
                    tag_size_restrict_str(eheader->tag_size_restrict));
            printf("%*s: %s\n", TITLE_WIDTH, "Text Restriction",
                    text_enc_restrict_str(eheader->text_enc_restrict));
            printf("%*s: %s\n", TITLE_WIDTH, "Text Size Restriction",
                    text_size_restrict_str(eheader->text_size_restrict));
            printf("%*s: %s\n", TITLE_WIDTH, "Image Restriction",
                    img_enc_restrict_str(eheader->img_enc_restrict));
            printf("%*s: %s\n", TITLE_WIDTH, "Image Size Restriction",
                    img_size_restrict_str(eheader->img_size_restrict));
        }
    }
}

// Print an id3v2 frame header
void print_id3v2_frame_header(struct id3v2_frame_header *fheader,
        int verbosity) {
    assert(fheader);

    if (verbosity > 0) {
        printf("%*s: %.*s\n", TITLE_WIDTH, "Frame ID", ID3V2_FRAME_ID_SIZE,
                fheader->id);
        printf("%*s: %"PRIu32" bytes\n", TITLE_WIDTH, "Frame Size",
                fheader->size);

        if (fheader->group_id_present) {
            printf("%*s: %"PRIu8"\n", TITLE_WIDTH, "Grouping Identifier",
                    fheader->group_id);
        }
        if (fheader->data_length_present) {
            printf("%*s: %"PRIu32"\n", TITLE_WIDTH, "Data Length",
                    fheader->data_len);
        }

    }

    if (verbosity > 1) {
        printf("%*s: %s\n", TITLE_WIDTH, "Tag Alter Discard",
                boolstr(fheader->tag_alter_pres));
        printf("%*s: %s\n", TITLE_WIDTH, "File Alter Discard",
                boolstr(fheader->file_alter_pres));
        printf("%*s: %s\n", TITLE_WIDTH, "Read Only",
                boolstr(fheader->read_only));

        printf("%*s: %s\n", TITLE_WIDTH, "Group Information",
                boolstr(fheader->group_id_present));
        printf("%*s: %s\n", TITLE_WIDTH, "Compression",
                boolstr(fheader->compressed));
        printf("%*s: %s\n", TITLE_WIDTH, "Encryption",
                boolstr(fheader->encrypted));
        printf("%*s: %s\n", TITLE_WIDTH, "Unsynchronization",
                boolstr(fheader->unsynchronized));
        printf("%*s: %s\n", TITLE_WIDTH, "Data Length Indicator",
                boolstr(fheader->data_length_present));
    }
}

// Print the string with the given encoding. len should be -1 for NULL
// terminated strings and the string length in bytes otherwise.
// Returns the number of bytes printed on success, -1 otherwise.
static int print_enc(const char *str, int len, enum id3v2_encoding enc) {
    UChar *text;
    UErrorCode uerr = U_ZERO_ERROR;
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
                textlen = len * 2 + 2;
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

// Print an AENC frame
static void print_AENC_frame(struct id3v2_frame_header *fheader,
        int verbosity) {
    struct id3v2_frame_AENC frame;
    const char *title;

    parse_AENC_frame(fheader->data, &frame);
    title = frame_title(fheader);

    printf("%*s: %s - %s\n", TITLE_WIDTH, title, "Owner", frame.owner_id);
    printf("%*s: %s - %"PRIu16"\n", TITLE_WIDTH, title, "Preview Start",
            frame.preview_start);
    printf("%*s: %s - %"PRIu16"\n", TITLE_WIDTH, title, "Preview Length",
            frame.preview_length);
    printf("%*s: %s - ", TITLE_WIDTH, title, "Encryption Info");
    print_bin(frame.encryption_info,
            fheader->data_len - strlen(frame.owner_id) - 5);
    printf("\n");
}

// Print an APIC frame
static void print_APIC_frame(struct id3v2_frame_header *fheader,
        int verbosity, int extract) {
    struct id3v2_frame_APIC frame;
    const char *title;
    char *picfile;

    parse_APIC_frame(fheader, &frame);
    title = frame_title(fheader);

    if (verbosity > 0) {
        printf("%*s: %s - %s\n", TITLE_WIDTH, title, "Encoding",
                encoding_str(frame.encoding));
    }
    printf("%*s: %s - %s\n", TITLE_WIDTH, title, "MIME Type", frame.mime_type);
    printf("%*s: %s - %s\n", TITLE_WIDTH, title, "Picture Type",
            pic_type_str(frame.picture_type));
    printf("%*s: %s - ", TITLE_WIDTH, title, "Description");
    print_enc(frame.description, -1, frame.encoding);
    printf("\n");

    if (extract) {
        picfile = write_tmpfile(frame.picture, frame.picture_len);
        if (picfile == NULL) {
            return;
        }
        printf("%*s: %s - %s\n", TITLE_WIDTH, title, "Saved To", picfile);
        free(picfile);
    } else {
        printf("%*s: %s\n", TITLE_WIDTH, title, "Use -e to extract picture");
    }
}

// Print a COMM frame
static void print_COMM_frame(struct id3v2_frame_header *fheader,
        int verbosity) {
    struct id3v2_frame_COMM frame;
    const char *title;

    parse_COMM_frame(fheader, &frame);
    title = frame_title(fheader);

    if (verbosity > 0) {
        printf("%*s: %s - %s\n", TITLE_WIDTH, title, "Encoding",
                encoding_str(frame.encoding));
    }
    printf("%*s: %s - %s\n", TITLE_WIDTH, title, "Language", frame.language);
    printf("%*s: %s - ", TITLE_WIDTH, title, "Description");
    print_enc(frame.content_descriptor, -1, frame.encoding);
    printf("\n");
    printf("%*s: %s - ", TITLE_WIDTH, title, "Comment");
    print_enc(frame.comment, frame.comment_len, frame.encoding);
    printf("\n");
}

// Print a PRIV frame
static void print_PRIV_frame(struct id3v2_frame_header *fheader,
        int verbosity) {
    const char *title;
    size_t len;

    title = frame_title(fheader);

    printf("%*s: %s - %s\n", TITLE_WIDTH, title, "Owner", fheader->data);
    printf("%*s: ", TITLE_WIDTH, title);
    len = strlen((char *)fheader->data) + 1;
    print_bin(fheader->data + len, fheader->data_len - len);
    printf("\n");
}

// Print a UFID frame
static void print_UFID_frame(struct id3v2_frame_header *fheader,
        int verbosity) {
    struct id3v2_frame_UFID frame;
    const char *title;

    parse_UFID_frame(fheader->data, &frame);
    title = frame_title(fheader);
    printf("%*s: %s - %s\n", TITLE_WIDTH, title, "Owner", frame.owner);
    printf("%*s: ", TITLE_WIDTH, title);
    print_bin(frame.id, fheader->data_len - strlen(frame.owner) - 1);
    printf("\n");
}

// Print any text frame except TXXX
static void print_text_frame(struct id3v2_frame_header *fheader,
        int verbosity) {
    const char *title = frame_title(fheader);
    struct id3v2_frame_text frame;

    parse_text_frame(fheader->data, &frame);
    if (verbosity > 0) {
        printf("%*s: %s - %s\n", TITLE_WIDTH, title, "Encoding",
                encoding_str(frame.encoding));
    }
    printf("%*s: ", TITLE_WIDTH, title);
    print_enc(frame.text, fheader->data_len - 1, frame.encoding);
    printf("\n");
}

// Print a TXXX frame
static void print_TXXX_frame(struct id3v2_frame_header *fheader,
        int verbosity) {
    const char *title = frame_title(fheader);
    struct id3v2_frame_TXXX frame;

    parse_TXXX_frame(fheader->data, &frame);
    if (verbosity > 0) {
        printf("%*s: %s - %s\n", TITLE_WIDTH, title, "Encoding",
                encoding_str(frame.encoding));
    }
    printf("%*s: ", TITLE_WIDTH, title);
    print_enc(frame.description, -1, frame.encoding);
    printf(" - ");
    print_enc(frame.value,
            fheader->data_len -
                    strlen_enc(frame.description, frame.encoding) - 1,
            frame.encoding);
    printf("\n");
}

// Print any URL frame except WXXX
static void print_url_frame(struct id3v2_frame_header *fheader,
        int verbosity) {
    printf("%*s: %.*s\n", TITLE_WIDTH, frame_title(fheader), fheader->data_len,
            (char *)fheader->data);
}

// Print a WXXX frame
static void print_WXXX_frame(struct id3v2_frame_header *fheader,
        int verbosity) {
    const char *title = frame_title(fheader);
    struct id3v2_frame_WXXX frame;

    parse_WXXX_frame(fheader->data, &frame);
    if (verbosity > 0) {
        printf("%*s: %s - %s\n", TITLE_WIDTH, title, "Encoding",
                encoding_str(frame.encoding));
    }
    printf("%*s: %s - ", TITLE_WIDTH, title, "Description");
    print_enc(frame.description, -1, frame.encoding);
    printf("\n");
    printf("%*s: %s - %.*s\n", TITLE_WIDTH, title, "URL",
            (int)(fheader->data_len -
                strlen_enc(frame.description, frame.encoding) - 1),
            frame.url);
}

// Print an id3v2 frame
void print_id3v2_frame(struct id3v2_frame_header *header,
        int verbosity, int extract) {
    if (!strcmp(header->id, ID3V2_FRAME_ID_AENC)) {
        print_AENC_frame(header, verbosity);
    } else if (!strcmp(header->id, ID3V2_FRAME_ID_APIC)) {
        print_APIC_frame(header, verbosity, extract);
    } else if (!strcmp(header->id, ID3V2_FRAME_ID_COMM)) {
        print_COMM_frame(header, verbosity);
    } else if (!strcmp(header->id, ID3V2_FRAME_ID_PRIV)) {
        print_PRIV_frame(header, verbosity);
    } else if (!strcmp(header->id, ID3V2_FRAME_ID_UFID)) {
        print_UFID_frame(header, verbosity);
    } else if (!strcmp(header->id, ID3V2_FRAME_ID_TXXX)) {
        print_TXXX_frame(header, verbosity);
    } else if (header->id[0] == 'T') {
        print_text_frame(header, verbosity);
    } else if (!strcmp(header->id, ID3V2_FRAME_ID_WXXX)) {
        print_WXXX_frame(header, verbosity);
    } else if (header->id[0] == 'W') {
        print_url_frame(header, verbosity);
    } else {
        printf("Support for frame %.*s not implemented yet\n",
                ID3V2_FRAME_ID_SIZE, header->id);
    }
    if (verbosity > 0) {
        printf("\n");
    }
}
