// Implementation of ID3v2 tag reading functions
// Copyright 2015 David Gloe.

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "id3v2.h"

#if DEBUG
#define debug(fmt, ...) \
    fprintf(stderr, "%s:%s:%d " fmt "\n", \
        __FILE__, __func__, __LINE__, ##__VA_ARGS__);
#else
#define debug(fmt, ...)
#endif

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
            memcpy(fmap + i, extheader, len);
            if (header->version > ID3V2_SUPPORTED_VERSION) {
                debug("Tag version %"PRIu8" higher than supported version %d",
                        header->version, ID3V2_SUPPORTED_VERSION);
                continue;
            } else if (header->flags & 0x0f) {
                debug("Tag flags %"PRIx8" invalid", header->flags);
                continue;
            } else if (!is_synchsafe(header->tag_size)) {
                debug("Tag size %"PRIx32" not synchsafe", header->tag_size);
                continue;
            }
            found_header = 1;
            i += len;
            break;
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
        memcpy(fmap + i, extheader, len);
        i += len;
        if (!is_synchsafe(extheader->size)) {
            debug("Extended header size %"PRIx32" not synchsafe",
                    extheader->size);
            munmap(fmap, st.st_size);
            return 1;
        } else if (extheader->flag_size != ID3V2_EXTENDED_FLAG_SIZE) {
            debug("Extended header flag size %"PRIu8" should be %d",
                    extheader->flag_size, ID3V2_EXTENDED_FLAG_SIZE);
            munmap(fmap, st.st_size);
            return 1;
        } else if (extheader->flags & 0x8F) {
            debug("Extended header flags %"PRIx8" invalid", extheader->flags);
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
            memcpy(fmap + i, extheader->flag_data, len);
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
    memcpy(fmap + i, *frame_data, len);
    i += len;

    // Finally read the footer
    if (header->flags & ID3V2_HEADER_FOOTER_BIT) {
        len = sizeof(struct id3v2_footer);
        if (i > st.st_size - len) {
            debug("Unexpected eof in footer");
            munmap(fmap, st.st_size);
            return 1;
        }
        memcpy(fmap + i, footer, len);
        i += len;
        if (strncmp(footer->id, ID3V2_FOOTER_IDENTIFIER,
                    ID3V2_FOOTER_ID_SIZE)) {
            debug("Footer ID %c%c%c should be %s", footer->id[0],
                    footer->id[1], footer->id[2],
                    ID3V2_FOOTER_IDENTIFIER);
            munmap(fmap, st.st_size);
            return 1;
        } else if (footer->version > ID3V2_SUPPORTED_VERSION) {
            debug("Tag version %"PRIu8" higher than supported version %d",
                    header->version, ID3V2_SUPPORTED_VERSION);
            munmap(fmap, st.st_size);
            return 1;
        } else if (header->flags & 0x0f) {
            debug("Tag flags %"PRIx8" invalid", header->flags);
            munmap(fmap, st.st_size);
            return 1;
        } else if (!is_synchsafe(header->tag_size)) {
            debug("Tag size %"PRIx32" not synchsafe", header->tag_size);
            munmap(fmap, st.st_size);
            return 1;
        }
    }

    return 0;
}

int get_id3v2_frame(uint8_t *frame_data, size_t frame_data_len, size_t *index,
        struct id3v2_frame_header **header, uint8_t **data) {
    struct id3v2_frame_header *headerp;
    uint32_t data_len;

    assert(frame_data);
    assert(index);
    assert(header);
    assert(data);

    // We've reached the end of the tag
    if (*index + sizeof(struct id3v2_frame_header) > frame_data_len) {
        return 1;
    }

    // Validate the frame header
    headerp = (struct id3v2_frame_header *)(frame_data + *index);
    if (!is_synchsafe(headerp->size)) {
        debug("Frame size %"PRIx32" not synchsafe", headerp->size);
        return 1;
    } if (headerp->status_flags & 0x8F) {
        debug("Frame status flags %"PRIx8" invalid", headerp->status_flags);
        return 1;
    } else if (headerp->format_flags & 0x0B) {
        debug("Frame format flags %"PRIx8" invalid", headerp->format_flags);
        return 1;
    }
    *index += sizeof(struct id3v2_frame_header);

    // Make sure the data fits
    data_len = from_synchsafe(headerp->size);
    if (*index + data_len > frame_data_len) {
        debug("Index %zu tag data %"PRIu32" overflows frame %zu",
                *index, data_len, frame_data_len);
        return 1;
    }

    *header = headerp;
    *data = frame_data + *index;
    *index += from_synchsafe(headerp->size);

    return 0;
}
