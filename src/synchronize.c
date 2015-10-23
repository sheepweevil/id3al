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

// Unsynchronize the given data
// If no changes are required, outdata will equal data
// Otherwise, outdata must be freed by the caller
void unsynchronize(uint8_t *data, size_t len, uint8_t **outdata,
        size_t *outlen) {
    size_t synchronizations = 0, i, j;

    // A simple two pass algorithm, could possibly be improved
    for (i = 0; i < len - 1; i++) {
        if (data[i] == 0xFF &&
                (data[i+1] == 0x00 || (data[i+1] & 0xE0) == 0xE0)) {
            synchronizations++;
        }
    }

    if (!synchronizations) {
        *outdata = data;
        *outlen = len;
        return;
    }

    *outdata = malloc(len + synchronizations);
    if (*outdata == NULL) {
        *outlen = 0;
        return;
    }

    *outlen = len + synchronizations;
    for (i = 0, j = 0; i < len && j < *outlen; i++, j++) {
        (*outdata)[j] = data[i];
        if (i < len - 1 && data[i] == 0xFF &&
                (data[i+1] == 0x00 || (data[i+1] & 0xE0) == 0xE0)) {
            j++;
            (*outdata)[j] = 0;
        }
    }
    assert(i == len && j == *outlen);
}

// Resynchronize the given data
// If no changes are required, outdata will equal data
// Otherwise, outdata must be freed by the caller
void resynchronize(uint8_t *data, size_t len, uint8_t **outdata,
        size_t *outlen) {
    size_t unsynchronizations = 0, i, j;

    // A simple two pass algorithm, could possibly be improved
    for (i = 0; i < len - 1; i++) {
        if (data[i] == 0xFF && data[i+1] == 0x00) {
            unsynchronizations++;
        }
    }

    if (!unsynchronizations) {
        *outdata = data;
        *outlen = len;
        return;
    }

    *outdata = malloc(len - unsynchronizations);
    if (*outdata == NULL) {
        *outlen = 0;
        return;
    }

    *outlen = len - unsynchronizations;
    for (i = 0, j = 0; i < len && j < *outlen; i++, j++) {
        if (i > 0 && data[i] == 0x00 && data[i-1] == 0xFF) {
            i++;
        }
        (*outdata)[j] = data[i];
    }
    assert(i == len && j == *outlen);
}

