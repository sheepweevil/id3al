// Test for compliance with the ID3v2 standard
// Copyright 2015 David Gloe.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../id3v2.h"

void check_type_sizes(void) {
    assert(sizeof(uint24_t) == 3);
    assert(sizeof(struct id3v2_header) == 10);
    assert(sizeof(struct id3v2_footer) == 10);
    assert(sizeof(struct id3v2_frame_header) == 10);
}

void check_synchsafe(void) {
    assert(to_synchsafe(0x0FFFFFFF) == 0x7F7F7F7F);
    assert(from_synchsafe(0x7F7F7F7F) == 0x0FFFFFFF);
    assert(to_synchsafe(0) == 0);
    assert(from_synchsafe(0) == 0);
    assert(to_synchsafe(0x00204080) == 0x01010100);
    assert(from_synchsafe(0x01010100) == 0x00204080);
    assert(is_synchsafe(0x7F7F7F7F));
    assert(!is_synchsafe(0x80000000));
}

void check_synchronize(void) {
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

int main() {
    check_type_sizes();
    check_synchsafe();
    check_synchronize();

    printf("Passed!\n");
    return 0;
}
