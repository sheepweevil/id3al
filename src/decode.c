// Implementation of ID3v2 tag reading functions
// Copyright 2015 David Gloe.

#include <assert.h>
#include <inttypes.h>
#include "id3v2.h"

#if DEBUG
#define debug(fmt, ...) \
    fprintf(stderr, "%s:%s:%d " fmt "\n", \
        __FILE__, __func__, __LINE__, __VA_ARGS__);
#else
#define debug(fmt, ...)
#endif

int get_id3v2_tag(FILE *fp, struct id3v2_header *header,
        struct id3v2_extended_header *extheader,
        uint8_t **frame_data, size_t *frame_data_len,
        struct id3v2_footer *footer) {
    assert(fp);
    assert(header);
    assert(extheader);
    assert(frame_data);
    assert(frame_data_len);
    assert(footer);

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
