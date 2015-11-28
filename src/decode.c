// Implementation of ID3v2 tag reading functions
// Copyright 2015 David Gloe.

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "zlib.h"
#include "id3v2.h"

// Parse raw data into a header
// Return 1 on success, 0 otherwise
static int parse_id3v2_header(const uint8_t *fdata, size_t *i,
        struct id3v2_header *header) {
    uint8_t flags;

    memcpy(header->id, fdata, ID3V2_HEADER_ID_SIZE);
    header->id[ID3V2_HEADER_ID_SIZE] = 0;
    *i += ID3V2_HEADER_ID_SIZE;
    header->version = fdata[*i];
    (*i)++;
    header->revision = fdata[*i];
    (*i)++;
    flags = fdata[*i];
    header->unsynchronization = flags &
            ID3V2_HEADER_UNSYNCHRONIZATION_BIT;
    header->extheader_present = flags &
            ID3V2_HEADER_EXTENDED_HEADER_BIT;
    header->experimental = flags & ID3V2_HEADER_EXPERIMENTAL_BIT;
    header->footer_present = flags & ID3V2_HEADER_FOOTER_BIT;
    (*i)++;
    header->tag_size = byte_swap_32(*(uint32_t *)(fdata + *i));
    *i += sizeof(uint32_t);
    if (header->version >= 4) {
        if (!is_synchsafe(header->tag_size)) {
            debug("Tag size %"PRIx32" not synchsafe",
                    header->tag_size);
            return 0;
        }
        header->tag_size = from_synchsafe(header->tag_size);
    }
    header->i = 0;
    return 1;
}

// Parse raw data into an extended header
// Return 1 on success, 0 otherwise
static int parse_id3v2_extended_header(uint8_t *fdata, size_t *i,
        struct id3v2_header *header) {
    uint8_t flags;
    struct id3v2_extended_header *extheader = &header->extheader;

    extheader->size = byte_swap_32(*(uint32_t *)(fdata + *i));
    if (header->version >= 4) {
        if (!is_synchsafe(extheader->size)) {
            debug("Extended header size %"PRIx32" not synchsafe",
                    extheader->size);
            return 0;
        }
        extheader->size = from_synchsafe(extheader->size);
    }
    *i += sizeof(uint32_t);
    extheader->flag_size = fdata[*i];
    (*i)++;
    flags = fdata[*i];
    extheader->update = flags & ID3V2_EXTENDED_HEADER_UPDATE_BIT;
    extheader->crc_present = flags & ID3V2_EXTENDED_HEADER_CRC_BIT;
    extheader->restrictions = flags &
            ID3V2_EXTENDED_HEADER_TAG_RESTRICTIONS_BIT;
    (*i)++;
    if (extheader->update) {
        if (fdata[*i] != 0) {
            debug("Update flag data length %"PRIu8" not 0", fdata[*i]);
            return 0;
        }
        (*i)++;
    }
    if (extheader->crc_present) {
        if (header->version >= 4) {
            if (fdata[*i] != 5) {
                debug("CRC flag data length %"PRIu8" not 5", fdata[*i]);
                return 0;
            }
            (*i)++;
            extheader->crc = byte_swap_32(*(uint32_t *)(fdata + *i));
            if (!is_synchsafe(extheader->crc)) {
                debug("Extended header crc %"PRIx32" not synchsafe",
                        extheader->crc);
                return 0;
            }
            extheader->crc = from_synchsafe(extheader->crc);
            *i += sizeof(uint32_t);
            extheader->crc += fdata[*i] << 29;
            (*i)++;
        } else {
            if (fdata[*i] != 4) {
                debug("CRC flag data length %"PRIu8" not 4", fdata[*i]);
                return 0;
            }
            (*i)++;
            extheader->crc = byte_swap_32(*(uint32_t *)(fdata + *i));
            *i += sizeof(uint32_t);
        }
    }
    if (extheader->restrictions) {
        if (fdata[*i] != 1) {
            debug("Restriction flag data length %"PRIu8" not 1", fdata[*i]);
            return 0;
        }
        (*i)++;
        flags = fdata[*i];
        extheader->tag_size_restrict = get_tag_size_restriction(flags);
        extheader->text_enc_restrict = get_text_encoding_restriction(flags);
        extheader->text_size_restrict = get_text_size_restriction(flags);
        extheader->img_enc_restrict = get_image_encoding_restriction(flags);
        extheader->img_size_restrict = get_image_size_restriction(flags);
        (*i)++;
    }

    return 1;
}

// Parse raw data into the footer
// Return 1 if successful, 0 otherwise
static int parse_id3v2_footer(uint8_t *fdata, size_t *i,
        struct id3v2_header *header) {
    uint8_t flags;
    struct id3v2_footer *footer = &header->footer;

    memcpy(footer->id, fdata + *i, ID3V2_FOOTER_ID_SIZE);
    footer->id[ID3V2_FOOTER_ID_SIZE] = 0;
    *i += ID3V2_FOOTER_ID_SIZE;
    footer->version = fdata[*i];
    (*i)++;
    footer->revision = fdata[*i];
    (*i)++;
    flags = fdata[*i];
    footer->unsynchronization = flags &
            ID3V2_HEADER_UNSYNCHRONIZATION_BIT;
    footer->extheader_present = flags &
            ID3V2_HEADER_EXTENDED_HEADER_BIT;
    footer->experimental = flags & ID3V2_HEADER_EXPERIMENTAL_BIT;
    footer->footer_present = flags & ID3V2_HEADER_FOOTER_BIT;
    (*i)++;
    footer->tag_size = byte_swap_32(*(uint32_t *)(fdata + *i));
    if (header->version >= 4) {
        if (!is_synchsafe(footer->tag_size)) {
            debug("Footer tag size %"PRIx32" not synchsafe",
                    footer->tag_size);
            return 0;
        }
        footer->tag_size = from_synchsafe(footer->tag_size);
    }
    return 1;
}

// Find and decode the next ID3v2 tag in the file
// Caller must free the frame data
// Return 1 if successful, 0 otherwise
int get_id3v2_tag(int fd, struct id3v2_header *header) {
    struct stat st;
    void *fmap;
    size_t i;
    ssize_t len;
    int found_header, ret;

    assert(header);

    // Map the fd for easy parsing
    ret = fstat(fd, &st);
    if (ret == -1) {
        debug("fstat failed: %m");
        return 0;
    }
    fmap = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (fmap == MAP_FAILED) {
        debug("mmap %zu bytes failed: %m", st.st_size);
        return 0;
    }

    // Search for the header
    for (i = 0; i < st.st_size - ID3V2_HEADER_ID_SIZE; i++) {
        if (!strncmp(fmap + i, ID3V2_FILE_IDENTIFIER, ID3V2_HEADER_ID_SIZE)) {
            // We've found a header, read it in and check it
            if (parse_id3v2_header(fmap, &i, header)) {
                found_header = 1;
                break;
            } else {
                continue;
            }
        }
    }
    if (!found_header) {
        debug("No tag found in file");
        munmap(fmap, st.st_size);
        return 0;
    }

    // Read the extended header if it exists
    if (header->extheader_present) {
        if (!parse_id3v2_extended_header(fmap, &i, header)) {
            munmap(fmap, st.st_size);
            return 0;
        }
    }

    // Next read the frame data
    len = header->tag_size;
    if (header->extheader_present) {
        len -= from_synchsafe(header->extheader.size);
    }
    if (header->footer_present) {
        len -= ID3V2_FOOTER_SIZE;
    }
    if (len <= 0) {
        debug("Frame data length %zd invalid", len);
        munmap(fmap, st.st_size);
        return 0;
    }
    if (i > st.st_size - len) {
        debug("Unexpected eof in frame data");
        munmap(fmap, st.st_size);
        return 0;
    }
    header->frame_data = malloc(len);
    if (header->frame_data == NULL) {
        debug("malloc %zd failed: %m", len);
        munmap(fmap, st.st_size);
        return 0;
    }
    header->frame_data_len = len;
    memcpy(header->frame_data, fmap + i, len);
    i += len;

    // Finally read the footer
    if (header->footer_present) {
        if (i > st.st_size - ID3V2_FOOTER_SIZE) {
            debug("Unexpected eof in footer");
            munmap(fmap, st.st_size);
            return 0;
        }
        if (strncmp(fmap + i, ID3V2_FOOTER_IDENTIFIER, ID3V2_FOOTER_ID_SIZE)) {
            debug("Expected footer not found");
            munmap(fmap, st.st_size);
            return 0;
        }
        if (!parse_id3v2_footer(fmap, &i, header)) {
            munmap(fmap, st.st_size);
            return 0;
        }
    }

    return verify_id3v2_header(header);
}

// Get the next id3v2 frame from the tag.
//
// idheader is a pointer the id3v2 header structure
// header will contain the next frame header information
//
// Returns 1 if a frame was retrieved successfully, 0 otherwise
int get_id3v2_frame(struct id3v2_header *idheader,
        struct id3v2_frame_header *header) {
    uint8_t *synchronized;
    uint8_t flags;
    size_t sync_len;
    int ret;
    uLongf uncompresslen;

    assert(idheader);
    assert(header);

    // We've reached the end of the tag
    if (idheader->i + ID3V2_FRAME_HEADER_SIZE > idheader->frame_data_len) {
        return 0;
    } else if (idheader->frame_data[idheader->i] == 0) {
        // We've found padding
        return 0;
    }

    // Parse frame header
    memcpy(header->id, idheader->frame_data + idheader->i,
            ID3V2_FRAME_ID_SIZE);
    header->id[ID3V2_FRAME_ID_SIZE] = 0;
    idheader->i += ID3V2_FRAME_ID_SIZE;
    header->size = byte_swap_32(*(uint32_t *)(idheader->frame_data +
                idheader->i));
    idheader->i += sizeof(uint32_t);
    if (idheader->version >= 4) {
        if (!is_synchsafe(header->size)) {
            debug("Frame size %"PRIx32" not synchsafe", header->size);
            return 0;
        }
        header->size = from_synchsafe(header->size);
    }

    // Read flags
    flags = idheader->frame_data[idheader->i];
    (idheader->i)++;
    header->tag_alter_pres = flags & ID3V2_FRAME_HEADER_TAG_ALTER_BIT;
    header->file_alter_pres = flags & ID3V2_FRAME_HEADER_FILE_ALTER_BIT;
    header->read_only = flags & ID3V2_FRAME_HEADER_READ_ONLY_BIT;
    flags = idheader->frame_data[idheader->i];
    (idheader->i)++;
    header->group_id_present = flags & ID3V2_FRAME_HEADER_GROUPING_BIT;
    header->compressed = flags & ID3V2_FRAME_HEADER_COMPRESSION_BIT;
    header->encrypted = flags & ID3V2_FRAME_HEADER_ENCRYPTION_BIT;
    header->unsynchronized = flags & ID3V2_FRAME_HEADER_UNSYNCHRONIZATION_BIT;
    header->data_length_present = flags & ID3V2_FRAME_HEADER_DATA_LENGTH_BIT;

    // Read the grouping id if it exists
    if (header->group_id_present) {
        header->group_id = idheader->frame_data[idheader->i];
        (idheader->i)++;
    }

    // Get the data length if it exists
    if (header->data_length_present) {
        header->data_len = byte_swap_32(*(uint32_t *)(idheader->frame_data +
                    idheader->i));
        idheader->i += sizeof(uint32_t);
        if (idheader->version >= 4) {
            if (!is_synchsafe(header->data_len)) {
                debug("Frame data length %"PRIx32" not synchsafe",
                        header->data_len);
                return 0;
            }
            header->data_len = from_synchsafe(header->data_len);
        }
    }

    // Make sure the data fits
    if (idheader->i + header->size > idheader->frame_data_len) {
        debug("Index %zu tag data %"PRIu32" overflows frame %zu",
                idheader->i, header->size, idheader->frame_data_len);
        return 0;
    }

    if (!verify_id3v2_frame_header(header)) {
        return 0;
    }

    // Resynchronize if needed
    if (header->unsynchronized || idheader->unsynchronization) {
        sync_len = resync_len(idheader->frame_data + idheader->i, header->size);
        synchronized = malloc(sync_len);
        if (synchronized == NULL) {
            debug("malloc %zu failed: %m", sync_len);
            return 0;
        }
        resynchronize(idheader->frame_data + idheader->i, header->size,
                synchronized);
    } else {
        sync_len = header->size;
        synchronized = malloc(sync_len);
        if (synchronized == NULL) {
            debug("malloc %zu failed: %m", sync_len);
            return 0;
        }
        memcpy(synchronized, idheader->frame_data + idheader->i, header->size);
    }

    // Uncompress if needed
    // Note verify_id3v2_frame_header ensures data length is present
    if (header->compressed) {
        header->data = malloc(header->data_len);
        if (header->data == NULL) {
            debug("malloc %"PRIu32" failed: %m", header->data_len);
            free(synchronized);
            return 0;
        }

        uncompresslen = header->data_len;
        ret = uncompress(header->data, &uncompresslen, synchronized, sync_len);
        free(synchronized);
        if (ret != Z_OK) {
            debug("uncompress failed: %s", zError(ret));
            free(header->data);
            return 0;
        } else if (uncompresslen != header->data_len) {
            debug("uncompressed length mismatch: %lu != %"PRIu32,
                    uncompresslen, header->data_len);
            free(header->data);
            return 0;
        }
    } else {
        header->data = synchronized;
        header->data_len = sync_len;
    }
    idheader->i += header->size;

    return 1;
}

enum id3v2_restriction_tag_size get_tag_size_restriction(uint8_t flags) {
    return (flags & ID3V2_RESTRICTION_TAG_SIZE_BITS) >> 6;
}

enum id3v2_restriction_text_encoding get_text_encoding_restriction(
        uint8_t flags) {
    return (flags & ID3V2_RESTRICTION_TEXT_ENCODING_BITS) >> 5;
}

enum id3v2_restriction_text_size get_text_size_restriction(uint8_t flags) {
    return (flags & ID3V2_RESTRICTION_TEXT_SIZE_BITS) >> 3;
}

enum id3v2_restriction_image_encoding get_image_encoding_restriction(
        uint8_t flags) {
    return (flags & ID3V2_RESTRICTION_IMAGE_ENCODING_BITS) >> 2;
}

enum id3v2_restriction_image_size get_image_size_restriction(uint8_t flags) {
    return (flags & ID3V2_RESTRICTION_IMAGE_SIZE_BITS);
}

int parse_AENC_frame(uint8_t *fdata, struct id3v2_frame_AENC *frame) {
    size_t i;
    frame->owner_id = (char *)fdata;
    i = strlen(frame->owner_id) + 1;
    frame->preview_start = *(uint16_t *)(fdata + i);
    i += sizeof(uint16_t);
    frame->preview_length = *(uint16_t *)(fdata + i);
    i += sizeof(uint16_t);
    frame->encryption_info = fdata + i;
    return 1;
}

int parse_APIC_frame(struct id3v2_frame_header *header,
        struct id3v2_frame_APIC *frame) {
    size_t i = 0;
    frame->encoding = header->data[i];
    i++;
    frame->mime_type = (char *)(header->data + i);
    i += strlen(frame->mime_type) + 1;
    frame->picture_type = header->data[i];
    i++;
    frame->description = (char *)(header->data + i);
    i += strlen_enc(frame->description, frame->encoding);
    frame->picture = header->data + i;
    frame->picture_len = header->data_len - i;
    return 1;
}

int parse_UFID_frame(uint8_t *fdata, struct id3v2_frame_UFID *frame) {
    frame->owner = (char *)fdata;
    frame->id = fdata + strlen(frame->owner) + 1;
    return 1;
}

// Parse frames T000-TZZZ, excluding TXXX
int parse_text_frame(uint8_t *fdata, struct id3v2_frame_text *frame) {
    frame->encoding = *fdata;
    frame->text = (char *)(fdata + 1);
    return 1;
}

int parse_TXXX_frame(uint8_t *fdata, struct id3v2_frame_TXXX *frame) {
    frame->encoding = *fdata;
    frame->description = (char *)(fdata + 1);
    frame->value = (char *)(fdata + strlen(frame->description) + 2);
    return 1;
}

// Parse frames W000-WZZZ, excluding WXXX
int parse_url_frame(uint8_t *fdata, struct id3v2_frame_url *frame) {
    frame->url = (char *)fdata;
    return 1;
}

int parse_WXXX_frame(uint8_t *fdata, struct id3v2_frame_WXXX *frame) {
    frame->encoding = *fdata;
    frame->description = (char *)(fdata + 1);
    frame->url = (char *)(fdata + 1 + strlen(frame->description));
    return 1;
}

