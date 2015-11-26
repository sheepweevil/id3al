// Implementation of ID3v2 verification functions
// Copyright 2015 David Gloe.

#include <inttypes.h>
#include <string.h>
#include "id3v2.h"

int verify_id3v2_header(struct id3v2_header *header) {
    if (strcmp(header->id, ID3V2_FILE_IDENTIFIER)) {
        debug("Tag ID %s should be %s", header->id, ID3V2_FILE_IDENTIFIER);
        return 0;
    } else if (header->version > ID3V2_SUPPORTED_VERSION) {
        debug("Tag version %"PRIu8" higher than supported version %d",
                header->version, ID3V2_SUPPORTED_VERSION);
        return 0;
    }
    return 1;
}

int verify_id3v2_extended_header(struct id3v2_header *header,
        struct id3v2_extended_header *extheader) {
    if (extheader->flag_size != ID3V2_EXTENDED_FLAG_SIZE) {
        debug("Extended header flag size %"PRIu8" should be %d",
                extheader->flag_size, ID3V2_EXTENDED_FLAG_SIZE);
        return 0;
    }
    return 1;
}

int verify_id3v2_frame_header(struct id3v2_header *header,
        struct id3v2_frame_header *fheader) {
    if (fheader->compressed && !fheader->data_length_present) {
        debug("Frame %.*s compression requires data length",
                ID3V2_FRAME_ID_SIZE, fheader->id);
        return 0;
    }
    return 1;
}

int verify_id3v2_footer(struct id3v2_footer *footer) {
    if (strncmp(footer->id, ID3V2_FOOTER_IDENTIFIER,
                ID3V2_FOOTER_ID_SIZE)) {
        debug("Footer ID %c%c%c should be %s", footer->id[0],
                footer->id[1], footer->id[2],
                ID3V2_FOOTER_IDENTIFIER);
        return 0;
    } else if (footer->version > ID3V2_SUPPORTED_VERSION) {
        debug("Footer version %"PRIu8" higher than supported version %d",
                footer->version, ID3V2_SUPPORTED_VERSION);
        return 0;
    } else if (footer->flags & 0x0f) {
        debug("Footer flags 0x%"PRIx8" invalid", footer->flags);
        return 0;
    } else if (footer->version > 3 && !is_synchsafe(footer->tag_size)) {
        debug("Footer size 0x%"PRIx32" not synchsafe", footer->tag_size);
        return 0;
    }
    return 1;
}
