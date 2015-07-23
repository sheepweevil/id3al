// Implementation of ID3v2 synchronization
// Copyright 2015 David Gloe.

#include "id3v2.h"

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
// Returns 0 if successful, 1 on error
int unsynchronize(uint8_t *data, size_t len, uint8_t **outdata, size_t *outlen);

// Resynchronize the given data
// If no changes are required, outdata will equal data
// Otherwise, outdata must be freed by the caller
// Returns 0 if successful, 1 on error
int resynchronize(uint8_t *data, size_t len, uint8_t **outdata, size_t *outlen);

