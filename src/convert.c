// Implementation of ID3v2 conversion functions
// Copyright 2015 David Gloe.

#include <string.h>
#include "id3v2.h"

// Convert a boolean value to a string
const char *boolstr(int b) {
    if (b) {
        return "True";
    }
    return "False";
}

// Get a descriptive title for a frame
const char *frame_title(struct id3v2_frame_header *fheader) {
    static const char *frames[] = {
        ID3V2_FRAME_ID_AENC, ID3V2_FRAME_ID_APIC, ID3V2_FRAME_ID_ASPI,
        ID3V2_FRAME_ID_COMM, ID3V2_FRAME_ID_COMR, ID3V2_FRAME_ID_ENCR,
        ID3V2_FRAME_ID_EQU2, ID3V2_FRAME_ID_ETCO, ID3V2_FRAME_ID_GEOB,
        ID3V2_FRAME_ID_GRID, ID3V2_FRAME_ID_LINK, ID3V2_FRAME_ID_MCDI,
        ID3V2_FRAME_ID_MLLT, ID3V2_FRAME_ID_OWNE, ID3V2_FRAME_ID_PRIV,
        ID3V2_FRAME_ID_PCNT, ID3V2_FRAME_ID_POPM, ID3V2_FRAME_ID_POSS,
        ID3V2_FRAME_ID_RBUF, ID3V2_FRAME_ID_RVA2, ID3V2_FRAME_ID_RVRB,
        ID3V2_FRAME_ID_SEEK, ID3V2_FRAME_ID_SIGN, ID3V2_FRAME_ID_SYLT,
        ID3V2_FRAME_ID_SYTC, ID3V2_FRAME_ID_TALB, ID3V2_FRAME_ID_TBPM,
        ID3V2_FRAME_ID_TCOM, ID3V2_FRAME_ID_TCON, ID3V2_FRAME_ID_TCOP,
        ID3V2_FRAME_ID_TDEN, ID3V2_FRAME_ID_TDLY, ID3V2_FRAME_ID_TDOR,
        ID3V2_FRAME_ID_TDRC, ID3V2_FRAME_ID_TDRL, ID3V2_FRAME_ID_TDTG,
        ID3V2_FRAME_ID_TENC, ID3V2_FRAME_ID_TEXT, ID3V2_FRAME_ID_TFLT,
        ID3V2_FRAME_ID_TIPL, ID3V2_FRAME_ID_TIT1, ID3V2_FRAME_ID_TIT2,
        ID3V2_FRAME_ID_TIT3, ID3V2_FRAME_ID_TKEY, ID3V2_FRAME_ID_TLAN,
        ID3V2_FRAME_ID_TLEN, ID3V2_FRAME_ID_TMCL, ID3V2_FRAME_ID_TMED,
        ID3V2_FRAME_ID_TMOO, ID3V2_FRAME_ID_TOAL, ID3V2_FRAME_ID_TOFN,
        ID3V2_FRAME_ID_TOLY, ID3V2_FRAME_ID_TOPE, ID3V2_FRAME_ID_TOWN,
        ID3V2_FRAME_ID_TPE1, ID3V2_FRAME_ID_TPE2, ID3V2_FRAME_ID_TPE3,
        ID3V2_FRAME_ID_TPE4, ID3V2_FRAME_ID_TPOS, ID3V2_FRAME_ID_TPRO,
        ID3V2_FRAME_ID_TPUB, ID3V2_FRAME_ID_TRCK, ID3V2_FRAME_ID_TRSN,
        ID3V2_FRAME_ID_TRSO, ID3V2_FRAME_ID_TSOA, ID3V2_FRAME_ID_TSOP,
        ID3V2_FRAME_ID_TSOT, ID3V2_FRAME_ID_TSRC, ID3V2_FRAME_ID_TSSE,
        ID3V2_FRAME_ID_TSST, ID3V2_FRAME_ID_TXXX, ID3V2_FRAME_ID_UFID,
        ID3V2_FRAME_ID_USER, ID3V2_FRAME_ID_USLT, ID3V2_FRAME_ID_WCOM,
        ID3V2_FRAME_ID_WCOP, ID3V2_FRAME_ID_WOAF, ID3V2_FRAME_ID_WOAR,
        ID3V2_FRAME_ID_WOAS, ID3V2_FRAME_ID_WORS, ID3V2_FRAME_ID_WPAY,
        ID3V2_FRAME_ID_WPUB, ID3V2_FRAME_ID_WXXX
    };
    static const char *titles[] = {
        "Audio Encryption",
        "Attached Picture",
        "Audio Seek Point Index",
        "Comments",
        "Commercial Info",
        "Encryption Method",
        "Equalization",
        "Event Timing",
        "Encapsulated Object",
        "Group Identification",
        "Linked Info",
        "Music CD",
        "MPEG Lookup Table",
        "Ownership",
        "Private",
        "Play Counter",
        "Popularimeter",
        "Position Sync",
        "Recommended Buffer Size",
        "Relative Volume Adjust",
        "Reverb",
        "Seek",
        "Signature",
        "Synchronized Lyrics",
        "Synchronized Tempo",
        "Album Title",
        "BPM",
        "Composer",
        "Content Type",
        "Copyright",
        "Encoding Time",
        "Playlist Delay",
        "Original Release Time",
        "Recording Time",
        "Release Time",
        "Tagging Time",
        "Encoded By",
        "Lyricist",
        "File Type",
        "Involved People",
        "Content Group",
        "Title",
        "Subtitle",
        "Initial Key",
        "Language",
        "Length",
        "Musician Credits List",
        "Media Type",
        "Mood",
        "Original Album Title",
        "Original Filename",
        "Original Lyricist",
        "Original Artist",
        "File Owner",
        "Lead Performer",
        "Accompaniment",
        "Conductor",
        "Interpreted By",
        "Part of a Set",
        "Produced Notice",
        "Publisher",
        "Track Number",
        "Radio Station Name",
        "Radio Station Owner",
        "Album Sort Order",
        "Performer Sort Order",
        "Title Sort Order",
        "ISRC Code",
        "Encoding Settings",
        "Set Subtitle",
        "Text Info",
        "Unique File ID",
        "Terms of Use",
        "Lyrics",
        "Commercial Webpage",
        "Copyright Webpage",
        "Audio Webpage",
        "Artist Webpage",
        "Audio Source Webpage",
        "Radio Station Webpage",
        "Payment Webpage",
        "Publisher Webpage",
        "Webpage"
    };
    size_t i;

    for (i = 0; i < sizeof(frames) / sizeof(const char *); i++) {
        if (!strcmp(fheader->id, frames[i])) {
            return titles[i];
        }
    }
    return fheader->id;
}

// Describe the text encoding used
const char *encoding_str(enum id3v2_encoding enc) {
    switch (enc) {
        case ID3V2_ENCODING_ISO_8859_1:
            return "ISO 8859-1";
        case ID3V2_ENCODING_UTF_16:
            return "UTF-16 with BOM";
        case ID3V2_ENCODING_UTF_16BE:
            return "UTF-16 without BOM";
        case ID3V2_ENCODING_UTF_8:
            return "UTF-8";
        default:
            break;
    }
    return "Unknown";
}

// Describe a tag size restriction
const char *tag_size_restrict_str(enum id3v2_restriction_tag_size res) {
    switch (res) {
        case ID3V2_RESTRICTION_TAG_SIZE_1MB:
            return "No more than 128 frames and 1MB tag size";
        case ID3V2_RESTRICTION_TAG_SIZE_128KB:
            return "No more than 64 frames and 128KB tag size";
        case ID3V2_RESTRICTION_TAG_SIZE_40KB:
            return "No more than 32 frames and 40KB tag size";
        case ID3V2_RESTRICTION_TAG_SIZE_4KB:
            return "No more than 32 frames and 4KB tag size";
        default:
            break;
    }
    return "Unknown";
}

// Describe a text encoding restriction
const char *text_enc_restrict_str(enum id3v2_restriction_text_encoding res) {
    switch (res) {
        case ID3V2_RESTRICTION_TEXT_ENCODING_NONE:
            return "No text encoding restrictions";
        case ID3V2_RESTRICTION_TEXT_ENCODING_BYTE:
            return "Text encoded with ISO-8859-1 or UTF-8";
        default:
            break;
    }
    return "Unknown";
}

// Describe a text size restriction
const char *text_size_restrict_str(enum id3v2_restriction_text_size res) {
    switch (res) {
        case ID3V2_RESTRICTION_TEXT_SIZE_NONE:
            return "No text size restrictions";
        case ID3V2_RESTRICTION_TEXT_SIZE_1024:
            return "No string is longer than 1024 characters";
        case ID3V2_RESTRICTION_TEXT_SIZE_128:
            return "No string is longer than 128 characters";
        case ID3V2_RESTRICTION_TEXT_SIZE_30:
            return "No string is longer than 30 characters";
        default:
            break;
    }
    return "Unknown";
}

// Describe an image encoding restriction
const char *img_enc_restrict_str(enum id3v2_restriction_image_encoding res) {
    switch (res) {
        case ID3V2_RESTRICTION_IMAGE_ENCODING_NONE:
            return "No image encoding restrictions";
        case ID3V2_RESTRICTION_IMAGE_ENCODING_COMPRESSED:
            return "Images encoded with PNG or JPEG";
        default:
            break;
    }
    return "Unknown";
}

// Describe an image size restriction
const char *img_size_restrict_str(enum id3v2_restriction_image_size res) {
    switch (res) {
        case ID3V2_RESTRICTION_IMAGE_SIZE_NONE:
            return "No image size restrictions";
        case ID3V2_RESTRICTION_IMAGE_SIZE_256:
            return "All images are 256x256 pixels or smaller";
        case ID3V2_RESTRICTION_IMAGE_SIZE_64:
            return "All images are 64x64 pixels or smaller";
        case ID3V2_RESTRICTION_IMAGE_SIZE_64_STRICT:
            return "All images are exactly 64x64 pixels";
        default:
            break;
    }
    return "Unknown";
}

// Describe a timestamp format for ETCO
const char *timestamp_fmt_str(enum id3v2_timestamp_format time_fmt) {
    switch (time_fmt) {
        case ID3V2_TIMESTAMP_FORMAT_MPEG:
            return "MPEG frames";
        case ID3V2_TIMESTAMP_FORMAT_MS:
            return "Milliseconds";
        default:
            break;
    }
    return "Unknown";
}

// Describe an ETCO event
const char *event_str(uint8_t event_type) {
    switch (event_type) {
        case ID3V2_EVENT_PADDING:
            return "Padding";
        case ID3V2_EVENT_INITIAL_SILENCE_END:
            return "End of initial silence";
        case ID3V2_EVENT_INTRO_START:
            return "Intro start";
        case ID3V2_EVENT_MAIN_PART_START:
            return "Main part start";
        case ID3V2_EVENT_OUTRO_START:
            return "Outro start";
        case ID3V2_EVENT_OUTRO_END:
            return "Outro end";
        case ID3V2_EVENT_VERSE_START:
            return "Verse start";
        case ID3V2_EVENT_REFRAIN_START:
            return "Refrain start";
        case ID3V2_EVENT_INTERLUDE_START:
            return "Interlude start";
        case ID3V2_EVENT_THEME_START:
            return "Theme start";
        case ID3V2_EVENT_VARIATION_START:
            return "Variation start";
        case ID3V2_EVENT_KEY_CHANGE:
            return "Key change";
        case ID3V2_EVENT_TIME_CHANGE:
            return "Time signature change";
        case ID3V2_EVENT_MOMENTARY_NOISE:
            return "Momentary unwanted noise";
        case ID3V2_EVENT_SUSTAINED_NOISE:
            return "Sustained noise";
        case ID3V2_EVENT_SUSTAINED_NOISE_END:
            return "Sustained noise end";
        case ID3V2_EVENT_INTRO_END:
            return "Intro end";
        case ID3V2_EVENT_MAIN_PART_END:
            return "Main part end";
        case ID3V2_EVENT_VERSE_END:
            return "Verse end";
        case ID3V2_EVENT_REFRAIN_END:
            return "Refrain end";
        case ID3V2_EVENT_THEME_END:
            return "Theme end";
        case ID3V2_EVENT_PROFANITY:
            return "Profanity";
        case ID3V2_EVENT_PROFANITY_END:
            return "Profanity end";
        case ID3V2_EVENT_AUDIO_END:
            return "Audio end";
        case ID3V2_EVENT_FILE_END:
            return "File end";
        case ID3V2_EVENT_EVENTS_FOLLOW:
            return "Events follow";
        default:
            break;
    }
    return "Reserved";
}

// Describe a synchronized text event
const char *sync_text_str(enum id3v2_SYLT_text text_type) {
    switch (text_type) {
        case ID3V2_SYLT_TEXT_OTHER:
            return "Other";
        case ID3V2_SYLT_TEXT_LYRICS:
            return "Lyrics";
        case ID3V2_SYLT_TEXT_TRANSCRIPTION:
            return "Transcription";
        case ID3V2_SYLT_TEXT_MOVEMENT:
            return "Movement name";
        case ID3V2_SYLT_TEXT_EVENT:
            return "Events";
        case ID3V2_SYLT_TEXT_CHORD:
            return "Chord";
        case ID3V2_SYLT_TEXT_TRIVIA:
            return "Trivia";
        case ID3V2_SYLT_TEXT_WEBPAGE:
            return "Webpage";
        case ID3V2_SYLT_TEXT_IMAGE:
            return "Image";
        default:
            break;
    }
    return "Unknown";
}

// Describe a channel
const char *channel_str(enum id3v2_RVA2_channel_type channel) {
    switch (channel) {
        case ID3V2_RVA2_CHANNEL_OTHER:
            return "Other";
        case ID3V2_RVA2_CHANNEL_MASTER:
            return "Master";
        case ID3V2_RVA2_CHANNEL_FRONT_RIGHT:
            return "Front Right";
        case ID3V2_RVA2_CHANNEL_FRONT_LEFT:
            return "Front Left";
        case ID3V2_RVA2_CHANNEL_BACK_RIGHT:
            return "Back Right";
        case ID3V2_RVA2_CHANNEL_BACK_LEFT:
            return "Back Left";
        case ID3V2_RVA2_CHANNEL_FRONT_CENTER:
            return "Front Center";
        case ID3V2_RVA2_CHANNEL_BACK_CENTER:
            return "Back Center";
        case ID3V2_RVA2_CHANNEL_SUBWOOFER:
            return "Subwoofer";
        default:
            break;
    }
    return "Unknown";
}

// Describe an interpolation method
const char *interp_str(enum id3v2_EQU2_interpolation_method interp) {
    switch (interp) {
        case ID3V2_EQU2_INTERPOLATION_BAND:
            return "No interpolation made";
        case ID3V2_EQU2_INTERPOLATION_LINEAR:
            return "Linear";
        default:
            break;
    }
    return "Unknown";
}

// Describe a picture type
const char *pic_type_str(enum id3v2_APIC_picture_type pic_type) {
    switch (pic_type) {
        case ID3V2_APIC_PICTURE_OTHER:
            return "Other";
        case ID3V2_APIC_PICTURE_FILE_ICON:
            return "File Icon";
        case ID3V2_APIC_PICTURE_FRONT_COVER:
            return "Front Cover";
        case ID3V2_APIC_PICTURE_BACK_COVER:
            return "Back Cover";
        case ID3V2_APIC_PICTURE_LEAFLET_PAGE:
            return "Leaflet Page";
        case ID3V2_APIC_PICTURE_MEDIA:
            return "Media";
        case ID3V2_APIC_PICTURE_LEAD_ARTIST:
            return "Lead Artist";
        case ID3V2_APIC_PICTURE_ARTIST:
            return "Artist";
        case ID3V2_APIC_PICTURE_CONDUCTOR:
            return "Conductor";
        case ID3V2_APIC_PICTURE_ORCHESTRA:
            return "Orchestra";
        case ID3V2_APIC_PICTURE_COMPOSER:
            return "Composer";
        case ID3V2_APIC_PICTURE_LYRICIST:
            return "Lyricist";
        case ID3V2_APIC_PICTURE_RECORDING_LOCATION:
            return "Recording Location";
        case ID3V2_APIC_PICTURE_RECORDING:
            return "During Recording";
        case ID3V2_APIC_PICTURE_PERFORMANCE:
            return "During Performance";
        case ID3V2_APIC_PICTURE_SCREEN_CAPTURE:
            return "Screen Capture";
        case ID3V2_APIC_PICTURE_BRIGHT_COLORED_FISH:
            return "A Bright Colored Fish";
        case ID3V2_APIC_PICTURE_ILLUSTRATION:
            return "Illustration";
        case ID3V2_APIC_PICTURE_BAND_LOGOTYPE:
            return "Band Logo";
        case ID3V2_APIC_PICTURE_PUBLISHER_LOGOTYPE:
            return "Publisher Logo";
        default:
            break;
    }
    return "Unknown";
}
