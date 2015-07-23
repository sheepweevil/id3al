// Test for compliance with the ID3v2 standard
// Copyright 2015 David Gloe.

#include <assert.h>
#include <stdio.h>
#include "../id3v2.h"

int main() {
    // Check type sizes
    assert(sizeof(uint24_t) == 3);
    assert(sizeof(struct id3v2_header) == 10);
    assert(sizeof(struct id3v2_footer) == 10);
    assert(sizeof(struct id3v2_frame_header) == 10);

    // Check synchsafe functions
    assert(to_synchsafe(0x0FFFFFFF) == 0x7F7F7F7F);
    assert(from_synchsafe(0x7F7F7F7F) == 0x0FFFFFFF);
    assert(to_synchsafe(0) == 0);
    assert(from_synchsafe(0) == 0);
    assert(to_synchsafe(0x00204080) == 0x01010100);
    assert(from_synchsafe(0x01010100) == 0x00204080);
    printf("Passed!\n");
    return 0;
}
