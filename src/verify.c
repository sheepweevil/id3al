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
    } else if (header->frame_data_len > 0 && header->frame_data == NULL) {
        debug("Tag frame data is NULL");
        return 0;
    } else if (header->frame_data_len > header->tag_size) {
        debug("Tag frame data len %zu > tag size %"PRIu32,
                header->frame_data_len, header->tag_size);
        return 0;
    } else if (header->frame_data_len && header->i >= header->frame_data_len) {
        debug("Tag frame data index %zu >= frame data len %zu",
                header->i, header->frame_data_len);
        return 0;
    }

    if (header->extheader_present &&
            header->extheader.flag_size != ID3V2_EXTENDED_FLAG_SIZE) {
        debug("Extended header flag size %"PRIu8" should be %d",
                header->extheader.flag_size, ID3V2_EXTENDED_FLAG_SIZE);
        return 0;
    }

    if (header->footer_present) {
        if (strcmp(header->footer.id, ID3V2_FOOTER_IDENTIFIER)) {
            debug("Footer ID %s should be %s",
                    header->footer.id, ID3V2_FOOTER_IDENTIFIER);
            return 0;
        } else if (header->footer.version > ID3V2_SUPPORTED_VERSION) {
            debug("Footer version %"PRIu8" higher than supported version %d",
                    header->footer.version, ID3V2_SUPPORTED_VERSION);
            return 0;
        } else if (header->footer.version != header->version) {
            debug("Footer version %"PRIu8" != header version %"PRIu8,
                    header->footer.version, header->version);
            return 0;
        } else if (header->footer.revision != header->revision) {
            debug("Footer revision %"PRIu8" != header revision %"PRIu8,
                    header->footer.revision, header->revision);
            return 0;
        } else if (header->footer.unsynchronization !=
                header->unsynchronization) {
            debug("Footer unsynchronization %s != header unsynchronization %s",
                    boolstr(header->footer.unsynchronization),
                    boolstr(header->unsynchronization));
            return 0;
        } else if (header->footer.extheader_present !=
                header->extheader_present) {
            debug("Footer ext header %s != header ext header %s",
                    boolstr(header->footer.extheader_present),
                    boolstr(header->extheader_present));
            return 0;
        } else if (header->footer.experimental != header->experimental) {
            debug("Footer experimental %s != header experimental %s",
                    boolstr(header->footer.experimental),
                    boolstr(header->experimental));
            return 0;
        } else if (header->footer.footer_present != header->footer_present) {
            debug("Footer footer %s != header footer %s",
                    boolstr(header->footer.footer_present),
                    boolstr(header->footer_present));
            return 0;
        }
    }
    return 1;
}

int verify_id3v2_frame_header(struct id3v2_frame_header *fheader) {
    if (fheader->compressed && !fheader->data_length_present) {
        debug("Frame %s compression requires data length", fheader->id);
        return 0;
    } else if (fheader->data_len > 0 && fheader->data == NULL) {
        debug("Frame %s data is NULL", fheader->id);
        return 0;
    }
    return 1;
}
