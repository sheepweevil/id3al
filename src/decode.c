// Implementation of ID3v2 tag reading functions
// Copyright 2015 David Gloe.

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "zlib.h"
#include "id3v2.h"

int get_id3v2_tag(int fd, struct id3v2_header *header,
        struct id3v2_extended_header *extheader,
        uint8_t **frame_data, size_t *frame_data_len,
        struct id3v2_footer *footer) {
    struct stat st;
    void *fmap;
    size_t i;
    ssize_t len;
    int found_header, ret;

    assert(header);
    assert(extheader);
    assert(frame_data);
    assert(frame_data_len);
    assert(footer);

    // Map the fd for easy parsing
    ret = fstat(fd, &st);
    if (ret == -1) {
        debug("fstat failed: %m");
        return 1;
    }
    fmap = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (fmap == MAP_FAILED) {
        debug("mmap %zu bytes failed: %m", st.st_size);
        return 1;
    }

    // Search for the header
    len = sizeof(struct id3v2_header);
    for (i = 0; i < st.st_size - len; i++) {
        if (!strncmp(fmap + i, ID3V2_FILE_IDENTIFIER, ID3V2_HEADER_ID_SIZE)) {
            // We've found a header, read it in and check it
            memcpy(header, fmap + i, len);
            header->tag_size = byte_swap_32(header->tag_size);
            i += len;
            if (!verify_id3v2_header(header)) {
                continue;
            } else {
                found_header = 1;
                break;
            }
        }
    }
    if (!found_header) {
        debug("No tag found in file");
        munmap(fmap, st.st_size);
        return 1;
    }

    // Read the extended header if it exists
    if (header->flags & ID3V2_HEADER_EXTENDED_HEADER_BIT) {
        len = ID3V2_EXTENDED_HEADER_MIN_SIZE;
        if (i > st.st_size - len) {
            debug("Unexpected eof in extended header size field");
            munmap(fmap, st.st_size);
            return 1;
        }
        memcpy(extheader, fmap + i, len);
        extheader->size = byte_swap_32(extheader->size);
        i += len;
        if (!verify_id3v2_extended_header(header, extheader)) {
            munmap(fmap, st.st_size);
            return 1;
        }

        len = from_synchsafe(extheader->size) - ID3V2_EXTENDED_HEADER_MIN_SIZE;
        if (len > 0) {
            if (i > st.st_size - len) {
                debug("Unexpected eof in extended header");
                munmap(fmap, st.st_size);
                return 1;
            }
            extheader->flag_data = malloc(len);
            if (extheader->flag_data == NULL) {
                debug("malloc %zu failed: %m", len);
                munmap(fmap, st.st_size);
                return 1;
            }
            memcpy(extheader->flag_data, fmap + i, len);
            i += len;
        }
    }

    // Next read the frame data
    len = from_synchsafe(header->tag_size);
    if (header->flags & ID3V2_HEADER_EXTENDED_HEADER_BIT) {
        len -= from_synchsafe(extheader->size);
    }
    if (header->flags & ID3V2_HEADER_FOOTER_BIT) {
        len -= sizeof(struct id3v2_footer);
    }
    if (len <= 0) {
        debug("Frame data length %zd invalid", len);
        munmap(fmap, st.st_size);
        return 1;
    }
    if (i > st.st_size - len) {
        debug("Unexpected eof in frame data");
        munmap(fmap, st.st_size);
        return 1;
    }
    *frame_data = malloc(len);
    if (*frame_data == NULL) {
        debug("malloc %zd failed: %m", len);
        munmap(fmap, st.st_size);
        return 1;
    }
    *frame_data_len = len;
    memcpy(*frame_data, fmap + i, len);
    i += len;

    // Finally read the footer
    if (header->flags & ID3V2_HEADER_FOOTER_BIT) {
        len = sizeof(struct id3v2_footer);
        if (i > st.st_size - len) {
            debug("Unexpected eof in footer");
            munmap(fmap, st.st_size);
            return 1;
        }
        memcpy(footer, fmap + i, len);
        footer->tag_size = byte_swap_32(footer->tag_size);
        i += len;


        if (!verify_id3v2_footer(footer)) {
            munmap(fmap, st.st_size);
            return 1;
        }
    }

    return 0;
}

// Get the next id3v2 frame from the tag.
//
// idheader is a pointer the id3v2 header structure
// frames is a pointer to the beginning of all frame data
// frames_len is the length in bytes of all frame data
// index should be initialized to 0 before the first call, then
//     remain unchanged by the caller
// header will contain the next frame header information
// group_id will contain the grouping identifier, if one is present
// frame_data will contain resynchronized, uncompressed frame data,
//     and must be freed by the caller
// frame_data_len will contain the length of the frame data
//
// Returns 1 if a frame was retrieved successfully, 0 otherwise
int get_id3v2_frame(struct id3v2_header *idheader, uint8_t *frames,
        size_t frames_len, size_t *index, struct id3v2_frame_header *header,
        uint8_t *group_id, uint8_t **frame_data, uint32_t *frame_data_len) {
    uint8_t *synchronized;
    size_t sync_len;
    int ret;
    uLongf uncompresslen;

    assert(idheader);
    assert(frames);
    assert(index);
    assert(header);
    assert(group_id);
    assert(frame_data);
    assert(frame_data_len);

    // We've reached the end of the tag
    if (*index + sizeof(struct id3v2_frame_header) > frames_len) {
        return 0;
    } else if (*(frames + *index) == 0) {
        // We've found padding
        return 0;
    }

    // Validate the frame header
    memcpy(header, frames + *index, sizeof(struct id3v2_frame_header));
    header->size = byte_swap_32(header->size);
    // Size isn't synchsafe in 2.3
    if (idheader->version > 3) {
        header->size = from_synchsafe(header->size);
    }
    if (!verify_id3v2_frame_header(idheader, header)) {
        return 0;
    }
    *index += sizeof(struct id3v2_frame_header);

    // Make sure the data fits
    if (*index + header->size > frames_len) {
        debug("Index %zu tag data %"PRIu32" overflows frame %zu",
                *index, header->size, frames_len);
        return 0;
    }

    // Read the grouping id if it exists
    if (header->format_flags & ID3V2_FRAME_HEADER_GROUPING_BIT) {
        *group_id = *(frames + *index);
        (*index)++;
    }

    // Get the data length if it exists
    if (header->format_flags & ID3V2_FRAME_HEADER_DATA_LENGTH_BIT) {
        *frame_data_len = from_synchsafe(byte_swap_32(*(uint32_t *)
                    (frames + *index)));
        (*index)++;
    }

    // Resynchronize if needed
    if (header->format_flags & ID3V2_FRAME_HEADER_UNSYNCHRONIZATION_BIT) {
        sync_len = resync_len(frames + *index, header->size);
        synchronized = malloc(sync_len);
        if (synchronized == NULL) {
            debug("malloc %zu failed: %m", sync_len);
            return 0;
        }
        resynchronize(frames + *index, header->size, synchronized);
    } else {
        sync_len = header->size;
        synchronized = malloc(sync_len);
        if (synchronized == NULL) {
            debug("malloc %zu failed: %m", sync_len);
            return 0;
        }
        memcpy(synchronized, frames + *index, header->size);
    }

    // Uncompress if needed
    // Note verify_id3v2_frame_header ensures data length is present
    if (header->format_flags & ID3V2_FRAME_HEADER_COMPRESSION_BIT) {
        *frame_data = malloc(*frame_data_len);
        if (*frame_data == NULL) {
            debug("malloc %"PRIu32" failed: %m", *frame_data_len);
            free(synchronized);
            return 0;
        }

        uncompresslen = *frame_data_len;
        ret = uncompress(*frame_data, &uncompresslen, synchronized, sync_len);
        free(synchronized);
        if (ret != Z_OK) {
            debug("uncompress failed: %s", zError(ret));
            free(*frame_data);
            return 0;
        } else if (uncompresslen != *frame_data_len) {
            debug("uncompressed length mismatch: %lu != %"PRIu32,
                    uncompresslen, *frame_data_len);
            free(*frame_data);
            return 0;
        }
    } else {
        *frame_data = synchronized;
        *frame_data_len = sync_len;
    }
    *index += header->size;

    return 1;
}

enum id3v2_restriction_tag_size get_tag_size_restriction(uint8_t flags) {
    return (flags & ID3V2_RESTRICTION_TAG_SIZE_BITS) >> 6;
}

enum id3v2_restriction_text_encoding get_text_encoding_restriction(
        uint8_t flags) {
    return (flags & ID3V2_RESTRICTION_TEXT_ENCODING_BITS) >> 5;
}

enum id3v2_restriction_text_size get_text_size_restriction(uint8_t flags) {
    return (flags & ID3V2_RESTRICTION_TEXT_SIZE_BITS) >> 3;
}

enum id3v2_restriction_image_encoding get_image_encoding_restriction(
        uint8_t flags) {
    return (flags & ID3V2_RESTRICTION_IMAGE_ENCODING_BITS) >> 2;
}

enum id3v2_restriction_image_size get_image_size_restriction(uint8_t flags) {
    return (flags & ID3V2_RESTRICTION_IMAGE_SIZE_BITS);
}

int parse_UFID_frame(uint8_t *fdata, struct id3v2_frame_UFID *frame) {
    frame->owner = (char *)fdata;
    frame->id = fdata + strlen(frame->owner) + 1;
    return 1;
}

// Parse frames T000-TZZZ, excluding TXXX
int parse_text_frame(uint8_t *fdata, struct id3v2_frame_text *frame) {
    frame->encoding = *fdata;
    frame->text = (char *)(fdata + 1);
    return 1;
}

int parse_TXXX_frame(uint8_t *fdata, struct id3v2_frame_TXXX *frame) {
    frame->encoding = *fdata;
    frame->description = (char *)(fdata + 1);
    frame->value = (char *)(fdata + 1 + strlen(frame->description));
    return 1;
}

// Parse frames W000-WZZZ, excluding WXXX
int parse_url_frame(uint8_t *fdata, struct id3v2_frame_url *frame) {
    frame->url = (char *)fdata;
    return 1;
}

int parse_WXXX_frame(uint8_t *fdata, struct id3v2_frame_WXXX *frame) {
    frame->encoding = *fdata;
    frame->description = (char *)(fdata + 1);
    frame->url = (char *)(fdata + 1 + strlen(frame->description));
    return 1;
}

