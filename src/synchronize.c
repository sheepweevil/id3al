// Implementation of ID3v2 synchronization
// Copyright 2015 David Gloe.

#include <assert.h>
#include <stdlib.h>
#include "id3v2.h"

// Determine whether an integer is synchsafe or not
int is_synchsafe(uint32_t val) {
    return !(val & 0x80808080);
}

// Convert a synchsafe integer to a normal one
uint32_t from_synchsafe(uint32_t val) {
    uint32_t ret;
    ret = val & 0x0000007F;
    val >>= 1;
    ret += val & 0x00003F80;
    val >>= 1;
    ret += val & 0x001FC000;
    val >>= 1;
    ret += val & 0x0FE00000;
    return ret;
}

// Convert a normal integer to a synchsafe one
// The input value must be less than 2^28
uint32_t to_synchsafe(uint32_t val) {
    uint32_t ret;
    ret = val & 0x0000007F;
    val <<= 1;
    ret += val & 0x00007F00;
    val <<= 1;
    ret += val & 0x007F0000;
    val <<= 1;
    ret += val & 0x7F000000;
    return ret;
}

// Determine the data length if it was unsynchronized
size_t unsync_len(const uint8_t *data, size_t len) {
    size_t synchronizations = 0, i;

    for (i = 0; i < len - 1; i++) {
        if (data[i] == 0xFF &&
                (data[i+1] == 0x00 || (data[i+1] & 0xE0) == 0xE0)) {
            synchronizations++;
        }
    }
    return len + synchronizations;
}

// Determine the data length if it was resynchronized
size_t resync_len(const uint8_t *data, size_t len) {
    size_t unsynchronizations = 0, i;

    // A simple two pass algorithm, could possibly be improved
    for (i = 0; i < len - 1; i++) {
        if (data[i] == 0xFF && data[i+1] == 0x00) {
            unsynchronizations++;
        }
    }
    return len - unsynchronizations;
}

// Unsynchronize the given data
// outdata must be at least unsync_len(data, len) bytes
void unsynchronize(const uint8_t *data, size_t len, uint8_t *outdata) {
    size_t i, j;

    for (i = 0, j = 0; i < len; i++, j++) {
        outdata[j] = data[i];
        if (i < len - 1 && data[i] == 0xFF &&
                (data[i+1] == 0x00 || (data[i+1] & 0xE0) == 0xE0)) {
            j++;
            outdata[j] = 0;
        }
    }
}

// Resynchronize the given data
// outdata must be at least resync_len(data, len) bytes
void resynchronize(const uint8_t *data, size_t len, uint8_t *outdata) {
    size_t i, j;

    for (i = 0, j = 0; i < len; i++, j++) {
        if (i > 0 && data[i] == 0x00 && data[i-1] == 0xFF) {
            i++;
        }
        outdata[j] = data[i];
    }
}

uint32_t byte_swap_32(uint32_t val) {
    return ((val & 0xFF000000) >> 24) + ((val & 0xFF0000) >> 8) +
        ((val & 0xFF00) << 8) + ((val & 0xFF) << 24);
}
