// Test for compliance with the ID3v2 standard
// Copyright 2015 David Gloe.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../id3v2.h"

static void check_synchsafe(void) {
    assert(to_synchsafe(0x0FFFFFFF) == 0x7F7F7F7F);
    assert(from_synchsafe(0x7F7F7F7F) == 0x0FFFFFFF);
    assert(to_synchsafe(0) == 0);
    assert(from_synchsafe(0) == 0);
    assert(to_synchsafe(0x00204080) == 0x01010100);
    assert(from_synchsafe(0x01010100) == 0x00204080);
    assert(is_synchsafe(0x7F7F7F7F));
    assert(!is_synchsafe(0x80000000));
}

static void check_byte_swap(void) {
    assert(byte_swap_32(0xAABBCCDD) == 0xDDCCBBAA);
}

static void check_synchronize(void) {
    uint8_t sync[4];
    uint8_t outsync[6];

    sync[0] = 0xFF;
    sync[1] = 0x0F;
    sync[2] = 0x00;
    sync[3] = 0xFF;
    assert(unsync_len(sync, sizeof(sync)) == sizeof(sync));
    unsynchronize(sync, sizeof(sync), outsync);
    assert(!memcmp(sync, outsync, sizeof(sync)));

    sync[0] = 0xFF;
    sync[1] = 0xF0;
    sync[2] = 0xFF;
    sync[3] = 0x00;
    assert(unsync_len(sync, sizeof(sync)) == 6);
    unsynchronize(sync, sizeof(sync), outsync);
    assert(outsync[0] == 0xFF && outsync[1] == 0x00 && outsync[2] == 0xF0 &&
            outsync[3] == 0xFF && outsync[4] == 0x00 && outsync[5] == 0x00);

    sync[0] = 0x00;
    sync[1] = 0xFF;
    sync[2] = 0x01;
    sync[3] = 0xFF;
    assert(resync_len(sync, sizeof(sync)) == sizeof(sync));
    resynchronize(sync, sizeof(sync), outsync);
    assert(!memcmp(sync, outsync, sizeof(sync)));

    sync[0] = 0x01;
    sync[1] = 0xFF;
    sync[2] = 0x00;
    sync[3] = 0x01;
    assert(resync_len(sync, sizeof(sync)) == 3);
    resynchronize(sync, sizeof(sync), outsync);
    assert(outsync[0] == 0x01 && outsync[1] == 0xFF && outsync[2] == 0x01);
}

static void check_verify(void) {
    struct id3v2_header header;
    struct id3v2_frame_header fheader;

    strncpy(header.id, ID3V2_FILE_IDENTIFIER, sizeof(header.id));
    header.version = 4;
    header.revision = 0;
    header.unsynchronization = 1;
    header.extheader_present = 1;
    header.experimental = 1;
    header.footer_present = 1;
    header.tag_size = 0x7f7f7f7f;
    header.extheader.size = 10;
    header.extheader.flag_size = 1;
    header.extheader.update = 0;
    header.extheader.crc_present = 1;
    header.extheader.restrictions = 1;
    header.extheader.crc = 0;
    header.frame_data = NULL;
    header.frame_data_len = 0;
    header.i = 0;
    strncpy(header.footer.id, ID3V2_FOOTER_IDENTIFIER,
            sizeof(header.footer.id));
    header.footer.version = 4;
    header.footer.revision = 0;
    header.footer.unsynchronization = 1;
    header.footer.extheader_present = 1;
    header.footer.experimental = 1;
    header.footer.footer_present = 1;
    header.footer.tag_size = 0x7f7f7f7f;

    assert(verify_id3v2_header(&header));

    header.id[2] = '4';
    assert(!verify_id3v2_header(&header));
    header.id[2] = '3';

    header.version = 5;
    assert(!verify_id3v2_header(&header));
    header.version = 4;

    header.extheader.flag_size = 2;
    assert(!verify_id3v2_header(&header));
    header.extheader.flag_size = 1;

    header.frame_data_len = 1;
    assert(!verify_id3v2_header(&header));
    header.frame_data_len = 0;

    header.footer.version = 5;
    assert(!verify_id3v2_header(&header));
    header.footer.version = 4;

    header.footer.id[0] = '4';
    assert(!verify_id3v2_header(&header));
    header.footer.id[0] = '3';

    header.footer.version = 1;
    assert(!verify_id3v2_header(&header));
    header.footer.version = 4;

    header.footer.revision = 1;
    assert(!verify_id3v2_header(&header));
    header.footer.revision = 0;

    header.footer.unsynchronization = 0;
    assert(!verify_id3v2_header(&header));
    header.footer.unsynchronization = 1;

    header.footer.extheader_present = 0;
    assert(!verify_id3v2_header(&header));
    header.footer.extheader_present = 1;

    header.footer.experimental = 0;
    assert(!verify_id3v2_header(&header));
    header.footer.experimental = 1;

    header.footer.footer_present = 0;
    assert(!verify_id3v2_header(&header));
    header.footer.footer_present = 1;

    strncpy(fheader.id, ID3V2_FRAME_ID_AENC, ID3V2_FRAME_ID_SIZE);
    fheader.size = 0x7f7f7f7f;
    fheader.compressed = 1;
    fheader.data_length_present = 1;
    fheader.data_len = 0;
    fheader.data = NULL;

    assert(verify_id3v2_frame_header(&fheader));

    fheader.data_len = 1;
    assert(!verify_id3v2_frame_header(&fheader));
    fheader.data_len = 0;

    fheader.data_length_present = 0;
}

static void check_conversion(void) {
    assert(get_tag_size_restriction(0xFF) == ID3V2_RESTRICTION_TAG_SIZE_4KB);
    assert(get_tag_size_restriction(0xBF) == ID3V2_RESTRICTION_TAG_SIZE_40KB);
    assert(get_tag_size_restriction(0x7F) == ID3V2_RESTRICTION_TAG_SIZE_128KB);
    assert(get_tag_size_restriction(0x3F) == ID3V2_RESTRICTION_TAG_SIZE_1MB);
    assert(get_text_encoding_restriction(0x20) ==
            ID3V2_RESTRICTION_TEXT_ENCODING_BYTE);
    assert(get_text_encoding_restriction(0x00) ==
            ID3V2_RESTRICTION_TEXT_ENCODING_NONE);
    assert(get_text_size_restriction(0x00) ==
            ID3V2_RESTRICTION_TEXT_SIZE_NONE);
    assert(get_text_size_restriction(0x08) ==
            ID3V2_RESTRICTION_TEXT_SIZE_1024);
    assert(get_text_size_restriction(0x10) ==
            ID3V2_RESTRICTION_TEXT_SIZE_128);
    assert(get_text_size_restriction(0x18) ==
            ID3V2_RESTRICTION_TEXT_SIZE_30);
    assert(get_image_encoding_restriction(0x00) ==
            ID3V2_RESTRICTION_IMAGE_ENCODING_NONE);
    assert(get_image_encoding_restriction(0x04) ==
            ID3V2_RESTRICTION_IMAGE_ENCODING_COMPRESSED);
    assert(get_image_size_restriction(0x00) ==
            ID3V2_RESTRICTION_IMAGE_SIZE_NONE);
    assert(get_image_size_restriction(0x01) ==
            ID3V2_RESTRICTION_IMAGE_SIZE_256);
    assert(get_image_size_restriction(0x02) ==
            ID3V2_RESTRICTION_IMAGE_SIZE_64);
    assert(get_image_size_restriction(0x03) ==
            ID3V2_RESTRICTION_IMAGE_SIZE_64_STRICT);
}

int main() {
    check_synchsafe();
    check_byte_swap();
    check_synchronize();
    check_verify();
    check_conversion();

    printf("Passed!\n");
    return 0;
}
