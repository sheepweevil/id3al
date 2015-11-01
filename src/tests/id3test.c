// Test for compliance with the ID3v2 standard
// Copyright 2015 David Gloe.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../id3v2.h"

static void check_type_sizes(void) {
    assert(sizeof(uint24_t) == 3);
    assert(sizeof(struct id3v2_header) == 10);
    assert(sizeof(struct id3v2_footer) == 10);
    assert(sizeof(struct id3v2_frame_header) == 10);
}

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
    uint8_t *outsync;
    size_t outlen;

    sync[0] = 0xFF;
    sync[1] = 0x0F;
    sync[2] = 0x00;
    sync[3] = 0xFF;
    unsynchronize(sync, sizeof(sync), &outsync, &outlen);
    assert(outsync == sync);
    assert(outlen == sizeof(sync));

    sync[0] = 0xFF;
    sync[1] = 0xF0;
    sync[2] = 0xFF;
    sync[3] = 0x00;
    unsynchronize(sync, sizeof(sync), &outsync, &outlen);
    assert(outlen == 6);
    assert(outsync[0] == 0xFF && outsync[1] == 0x00 && outsync[2] == 0xF0 &&
            outsync[3] == 0xFF && outsync[4] == 0x00 && outsync[5] == 0x00);
    free(outsync);

    sync[0] = 0x00;
    sync[1] = 0xFF;
    sync[2] = 0x01;
    sync[3] = 0xFF;
    resynchronize(sync, sizeof(sync), &outsync, &outlen);
    assert(outsync == sync);
    assert(outlen == sizeof(sync));

    sync[0] = 0x01;
    sync[1] = 0xFF;
    sync[2] = 0x00;
    sync[3] = 0x01;
    resynchronize(sync, sizeof(sync), &outsync, &outlen);
    assert(outlen == 3);
    assert(outsync[0] == 0x01 && outsync[1] == 0xFF && outsync[2] == 0x01);
    free(outsync);
}

static void check_verify(void) {
    struct id3v2_header header;
    struct id3v2_extended_header extheader;
    struct id3v2_footer footer;
    uint8_t flag_data[8];

    memcpy(header.id, ID3V2_FILE_IDENTIFIER, sizeof(header.id));
    header.version = 4;
    header.revision = 0;
    header.flags = 0xf0;
    header.tag_size = 0x7f7f7f7f;

    assert(verify_id3v2_header(&header));

    header.id[2] = '4';
    assert(!verify_id3v2_header(&header));
    header.id[2] = '3';

    header.version = 5;
    assert(!verify_id3v2_header(&header));
    header.version = 4;

    header.flags = 0x0f;
    assert(!verify_id3v2_header(&header));
    header.flags = 0xf0;

    header.tag_size = 0x80808080;
    assert(!verify_id3v2_header(&header));

    extheader.size = 0x7f7f7f7f;
    extheader.flag_size = 1;
    extheader.flags = 0x70;
    memset(flag_data, 0, sizeof(flag_data));
    extheader.flag_data = flag_data;

    assert(verify_id3v2_extended_header(&extheader));

    extheader.size = 0x80808080;
    assert(!verify_id3v2_extended_header(&extheader));
    extheader.size = 0x7f7f7f7f;

    extheader.flag_size = 2;
    assert(!verify_id3v2_extended_header(&extheader));
    extheader.flag_size = 1;

    extheader.flags = 0x8f;
    assert(!verify_id3v2_extended_header(&extheader));
    extheader.flags = 0x70;

    memcpy(footer.id, ID3V2_FOOTER_IDENTIFIER, sizeof(footer.id));
    footer.version = 4;
    footer.revision = 0;
    footer.flags = 0xf0;
    footer.tag_size = 0x7f7f7f7f;

    assert(verify_id3v2_footer(&footer));

    footer.id[2] = 'H';
    assert(!verify_id3v2_footer(&footer));
    footer.id[2] = 'I';

    footer.version = 5;
    assert(!verify_id3v2_footer(&footer));
    footer.version = 4;

    footer.flags = 0x0f;
    assert(!verify_id3v2_footer(&footer));
    footer.flags = 0xf0;

    footer.tag_size = 0x80808080;
    assert(!verify_id3v2_footer(&footer));
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
    check_type_sizes();
    check_synchsafe();
    check_byte_swap();
    check_synchronize();
    check_verify();
    check_conversion();

    printf("Passed!\n");
    return 0;
}
