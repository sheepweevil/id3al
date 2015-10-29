// Implementation of the id3al command
// Copyright 2015 David Gloe

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "id3v2.h"

static void print_id3v2_header(struct id3v2_header *header, int verbosity) {
    if (verbosity > 0) {
        printf("Tag header: ID %c%c%c version 2.%"PRIu8".%"PRIu8
                " size %"PRIu32" bytes\n",
                header->id[0], header->id[1], header->id[2], header->version,
                header->revision, from_synchsafe(header->tag_size));
    }

    if (verbosity > 1) {
        if (header->flags & ID3V2_HEADER_UNSYNCHRONIZATION_BIT) {
            printf("Unsynchronization applied to all frames\n");
        } else {
            printf("Unsynchronization not applied to all frames\n");
        }
        if (header->flags & ID3V2_HEADER_EXTENDED_HEADER_BIT) {
            printf("Extended header present\n");
        } else {
            printf("Extended header not present\n");
        }
        if (header->flags & ID3V2_HEADER_EXPERIMENTAL_BIT) {
            printf("Tag is in experimental stage\n");
        } else {
            printf("Tag is not in experimental stage\n");
        }
        if (header->flags & ID3V2_HEADER_FOOTER_BIT) {
            printf("Footer present\n");
        } else {
            printf("Footer not present\n");
        }
    }

    if (verbosity > 0) {
        printf("\n");
    }
    return;
}

static void print_id3v2_extended_header(struct id3v2_extended_header *eheader,
        int verbosity) {
    size_t i = 0;
    uint32_t crc;

    if (verbosity > 0) {
        printf("Extended header: size %"PRIu32"\n",
                from_synchsafe(eheader->size));
    }

    if (verbosity > 1) {
        if (eheader->flags & ID3V2_EXTENDED_HEADER_UPDATE_BIT) {
            printf("Tag is an update\n");
            i++;
        }
        if (eheader->flags & ID3V2_EXTENDED_HEADER_CRC_BIT) {
            i++;
            crc = eheader->flag_data[i] << 28;
            i++;
            crc += from_synchsafe(*(uint32_t *)(eheader->flag_data + i));
            printf("Tag CRC-32: %"PRIu32"\n", crc);
        }
        if (eheader->flags & ID3V2_EXTENDED_HEADER_TAG_RESTRICTIONS_BIT) {
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

int main(int argc, const char *argv[]) {
    struct id3v2_header header;
    struct id3v2_extended_header extheader;
    uint8_t flag_data[8];
    
    memcpy(header.id, ID3V2_FILE_IDENTIFIER, sizeof(header.id));
    header.version = 4;
    header.revision = 0;
    header.flags = 0xf0;
    header.tag_size = 0x7f7f7f7f;
    
    extheader.size = 0x7f7f7f7f;
    extheader.flag_size = 1;
    extheader.flags = 0x70;
    memset(flag_data, 0, sizeof(flag_data));
   
    print_id3v2_header(&header, 2);
    print_id3v2_extended_header(&extheader, 2);
    return 0;
}
