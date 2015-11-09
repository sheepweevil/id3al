// Implementation of ID3v2 verification functions
// Copyright 2015 David Gloe.

#include <inttypes.h>
#include <string.h>
#include "id3v2.h"

int verify_id3v2_header(struct id3v2_header *header) {
    if (strncmp(header->id, ID3V2_FILE_IDENTIFIER, ID3V2_HEADER_ID_SIZE)) {
        debug("Tag ID %.*s should be %s",
                ID3V2_HEADER_ID_SIZE, header->id, ID3V2_FILE_IDENTIFIER);
        return 0;
    } else if (header->version > ID3V2_SUPPORTED_VERSION) {
        debug("Tag version %"PRIu8" higher than supported version %d",
                header->version, ID3V2_SUPPORTED_VERSION);
        return 0;
    } else if (header->flags & 0x0f) {
        debug("Tag flags 0x%"PRIx8" invalid", header->flags);
        return 0;
    } else if (header->version > 3 && !is_synchsafe(header->tag_size)) {
        debug("Tag size 0x%"PRIx32" not synchsafe", header->tag_size);
        return 0;
    }
    return 1;
}

int verify_id3v2_extended_header(struct id3v2_header *header,
        struct id3v2_extended_header *extheader) {
    size_t i = 0;

    if (header->version > 3 && !is_synchsafe(extheader->size)) {
        debug("Extended header size 0x%"PRIx32" not synchsafe",
                extheader->size);
        return 0;
    } else if (extheader->flag_size != ID3V2_EXTENDED_FLAG_SIZE) {
        debug("Extended header flag size %"PRIu8" should be %d",
                extheader->flag_size, ID3V2_EXTENDED_FLAG_SIZE);
        return 0;
    } else if (extheader->flags & 0x8F) {
        debug("Extended header flags 0x%"PRIx8" invalid", extheader->flags);
        return 0;
    }
    if (extheader->flags & ID3V2_EXTENDED_HEADER_UPDATE_BIT) {
        if (extheader->flag_data[i] != 0) {
            debug("Extended header update length %"PRIu8" not 0",
                    extheader->flag_data[i]);
            return 0;
        }
        i++;
    }
    // TODO: check other flags
    return 1;
}

int verify_id3v2_frame_header(struct id3v2_header *header,
        struct id3v2_frame_header *fheader) {
    if (header->version > 3 && !is_synchsafe(fheader->size)) {
        debug("Frame %.*s size 0x%"PRIx32" not synchsafe",
                ID3V2_FRAME_ID_SIZE, fheader->id, fheader->size);
        return 0;
    } else if (fheader->status_flags & 0x8f) {
        debug("Frame %.*s status flags 0x%"PRIx8" invalid",
                ID3V2_FRAME_ID_SIZE, fheader->id, fheader->status_flags);
        return 0;
    } else if (fheader->format_flags & 0xb0) {
        debug("Frame %.*s format flags 0x%"PRIx8" invalid",
                ID3V2_FRAME_ID_SIZE, fheader->id, fheader->format_flags);
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
