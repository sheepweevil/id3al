// Implementation of ID3v2 tag reading functions
// Copyright 2015 David Gloe.

#include <assert.h>
#include "id3v2.h"

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
        struct id3v2_frame_header *header, uint8_t **data) {
    assert(frame_data);
    assert(index);
    assert(header);
    assert(data);

    return 0;
}
