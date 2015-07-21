// Test for compliance with the ID3v2 standard
// Copyright 2015 David Gloe.

#include <assert.h>
#include "../id3v2.h"

int main() {
    assert(sizeof(uint24_t) == 3);
    assert(sizeof(struct id3v2_header) == 10);
    assert(sizeof(struct id3v2_footer) == 10);
    assert(sizeof(struct id3v2_frame_header) == 10);
    return 0;
}
