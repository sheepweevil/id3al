// Header file with useful definitions for working with ID3v2.
// Copyright 2015 David Gloe.
//
// Protocol information from:
// http://id3.org/Developer%20Information

#ifndef _ID3V2_H
#define _ID3V2_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

// Used for 3 byte integer values
typedef struct uint24 { uint8_t byte[3]; } uint24_t;

// Header
#define ID3V2_FILE_IDENTIFIER   "ID3"

#define ID3V2_HEADER_UNSYNCHRONIZATION_BIT 0x80
#define ID3V2_HEADER_EXTENDED_HEADER_BIT   0x40
#define ID3V2_HEADER_EXPERIMENTAL_BIT      0x20
#define ID3V2_HEADER_FOOTER_BIT            0x10

#define ID3V2_HEADER_ID_SIZE 3
#define ID3V2_SUPPORTED_VERSION 4

struct id3v2_header {
    char     id[ID3V2_HEADER_ID_SIZE]; // not null terminated
    uint8_t  version;
    uint8_t  revision;
    uint8_t  flags;
    uint32_t tag_size; // synchsafe
} __attribute__((packed));

// Extended header
#define ID3V2_EXTENDED_HEADER_MIN_SIZE 6
#define ID3V2_EXTENDED_FLAG_SIZE 0x01

#define ID3V2_EXTENDED_HEADER_UPDATE_BIT           0x40
#define ID3V2_EXTENDED_HEADER_CRC_BIT              0x20
#define ID3V2_EXTENDED_HEADER_TAG_RESTRICTIONS_BIT 0x10

#define ID3V2_RESTRICTION_TAG_SIZE_BITS       0xC0
#define ID3V2_RESTRICTION_TEXT_ENCODING_BITS  0x20
#define ID3V2_RESTRICTION_TEXT_SIZE_BITS      0x18
#define ID3V2_RESTRICTION_IMAGE_ENCODING_BITS 0x04
#define ID3V2_RESTRICTION_IMAGE_SIZE_BITS     0x03

enum id3v2_restriction_tag_size {
    ID3V2_RESTRICTION_TAG_SIZE_1MB,   // 128 frames
    ID3V2_RESTRICTION_TAG_SIZE_128KB, // 64 frames
    ID3V2_RESTRICTION_TAG_SIZE_40KB,  // 32 frames
    ID3V2_RESTRICTION_TAG_SIZE_4KB    // 32 frames
};

enum id3v2_restriction_text_encoding {
    ID3V2_RESTRICTION_TEXT_ENCODING_NONE,
    ID3V2_RESTRICTION_TEXT_ENCODING_BYTE // ISO-8859-1 or UTF-8
};

enum id3v2_restriction_text_size {
    ID3V2_RESTRICTION_TEXT_SIZE_NONE,
    ID3V2_RESTRICTION_TEXT_SIZE_1024,
    ID3V2_RESTRICTION_TEXT_SIZE_128,
    ID3V2_RESTRICTION_TEXT_SIZE_30
};

enum id3v2_restriction_image_encoding {
    ID3V2_RESTRICTION_IMAGE_ENCODING_NONE,
    ID3V2_RESTRICTION_IMAGE_ENCODING_COMPRESSED // PNG or JPEG
};

enum id3v2_restriction_image_size {
    ID3V2_RESTRICTION_IMAGE_SIZE_NONE,
    ID3V2_RESTRICTION_IMAGE_SIZE_256,      // 256x256 pixels or smaller
    ID3V2_RESTRICTION_IMAGE_SIZE_64,       // 64x64 pixels or smaller
    ID3V2_RESTRICTION_IMAGE_SIZE_64_STRICT // 64x64 pixels exactly
};

struct id3v2_extended_header {
    uint32_t size;      // synchsafe
    uint8_t  flag_size;
    uint8_t  flags;
    uint8_t *flag_data;
};

// Footer
#define ID3V2_FOOTER_IDENTIFIER "3DI"

#define ID3V2_FOOTER_UNSYNCHRONIZATION_BIT 0x80
#define ID3V2_FOOTER_EXTENDED_HEADER_BIT   0x40
#define ID3V2_FOOTER_EXPERIMENTAL_BIT      0x20
#define ID3V2_FOOTER_FOOTER_BIT            0x10

#define ID3V2_FOOTER_ID_SIZE 3

struct id3v2_footer {
    char     id[ID3V2_FOOTER_ID_SIZE]; // not null terminated
    uint8_t  version;
    uint8_t  revision;
    uint8_t  flags;
    uint32_t tag_size; // synchsafe
} __attribute__((packed));

// Frame header
#define ID3V2_FRAME_HEADER_TAG_ALTER_BIT  0x40
#define ID3V2_FRAME_HEADER_FILE_ALTER_BIT 0x20
#define ID3V2_FRAME_HEADER_READ_ONLY_BIT  0x10

#define ID3V2_FRAME_HEADER_GROUPING_BIT          0x40
#define ID3V2_FRAME_HEADER_COMPRESSION_BIT       0x08
#define ID3V2_FRAME_HEADER_ENCRYPTION_BIT        0x04
#define ID3V2_FRAME_HEADER_UNSYNCHRONIZATION_BIT 0x02
#define ID3V2_FRAME_HEADER_DATA_LENGTH_BIT       0x01

#define ID3V2_FRAME_ID_SIZE 4

struct id3v2_frame_header {
    char     id[ID3V2_FRAME_ID_SIZE]; // not null terminated
    uint32_t size;                    // synchsafe
    uint8_t status_flags;
    uint8_t format_flags;
} __attribute__((packed));

// Encodings
enum id3v2_encoding {
    ID3V2_ENCODING_ISO_8859_1,
    ID3V2_ENCODING_UTF_16,
    ID3V2_ENCODING_UTF_16BE,
    ID3V2_ENCODING_UTF_8
};

// Frames
#define ID3V2_FRAME_ID_AENC "AENC" // Audio encryption
#define ID3V2_FRAME_ID_APIC "APIC" // Attached picture
#define ID3V2_FRAME_ID_ASPI "ASPI" // Audio seek point index
#define ID3V2_FRAME_ID_COMM "COMM" // Comments
#define ID3V2_FRAME_ID_COMR "COMR" // Commercial frame
#define ID3V2_FRAME_ID_ENCR "ENCR" // Encryption method registration
#define ID3V2_FRAME_ID_EQU2 "EQU2" // Equalisation (2)
#define ID3V2_FRAME_ID_ETCO "ETCO" // Event timing codes
#define ID3V2_FRAME_ID_GEOB "GEOB" // General encapsulated object
#define ID3V2_FRAME_ID_GRID "GRID" // Group identification registration
#define ID3V2_FRAME_ID_LINK "LINK" // Linked information
#define ID3V2_FRAME_ID_MCDI "MCDI" // Music CD identifier
#define ID3V2_FRAME_ID_MLLT "MLLT" // MPEG location lookup table
#define ID3V2_FRAME_ID_OWNE "OWNE" // Ownership frame
#define ID3V2_FRAME_ID_PRIV "PRIV" // Private frame
#define ID3V2_FRAME_ID_PCNT "PCNT" // Play counter
#define ID3V2_FRAME_ID_POPM "POPM" // Popularimeter
#define ID3V2_FRAME_ID_POSS "POSS" // Position synchronisation frame
#define ID3V2_FRAME_ID_RBUF "RBUF" // Recommended buffer size
#define ID3V2_FRAME_ID_RVA2 "RVA2" // Relative volume adjustment (2)
#define ID3V2_FRAME_ID_RVRB "RVRB" // Reverb
#define ID3V2_FRAME_ID_SEEK "SEEK" // Seek frame
#define ID3V2_FRAME_ID_SIGN "SIGN" // Signature frame
#define ID3V2_FRAME_ID_SYLT "SYLT" // Synchronised lyric/text
#define ID3V2_FRAME_ID_SYTC "SYTC" // Synchronised tempo codes
#define ID3V2_FRAME_ID_TALB "TALB" // Album/Movie/Show title
#define ID3V2_FRAME_ID_TBPM "TBPM" // BPM (beats per minute)
#define ID3V2_FRAME_ID_TCOM "TCOM" // Composer
#define ID3V2_FRAME_ID_TCON "TCON" // Content type
#define ID3V2_FRAME_ID_TCOP "TCOP" // Copyright message
#define ID3V2_FRAME_ID_TDEN "TDEN" // Encoding time
#define ID3V2_FRAME_ID_TDLY "TDLY" // Playlist delay
#define ID3V2_FRAME_ID_TDOR "TDOR" // Original release time
#define ID3V2_FRAME_ID_TDRC "TDRC" // Recording time
#define ID3V2_FRAME_ID_TDRL "TDRL" // Release time
#define ID3V2_FRAME_ID_TDTG "TDTG" // Tagging time
#define ID3V2_FRAME_ID_TENC "TENC" // Encoded by
#define ID3V2_FRAME_ID_TEXT "TEXT" // Lyricist/Text writer
#define ID3V2_FRAME_ID_TFLT "TFLT" // File type
#define ID3V2_FRAME_ID_TIPL "TIPL" // Involved people list
#define ID3V2_FRAME_ID_TIT1 "TIT1" // Content group description
#define ID3V2_FRAME_ID_TIT2 "TIT2" // Title/songname/content description
#define ID3V2_FRAME_ID_TIT3 "TIT3" //  Subtitle/Description refinement
#define ID3V2_FRAME_ID_TKEY "TKEY" // Initial key
#define ID3V2_FRAME_ID_TLAN "TLAN" // Language(s)
#define ID3V2_FRAME_ID_TLEN "TLEN" // Length
#define ID3V2_FRAME_ID_TMCL "TMCL" // Musician credits list
#define ID3V2_FRAME_ID_TMED "TMED" // Media type
#define ID3V2_FRAME_ID_TMOO "TMOO" // Mood
#define ID3V2_FRAME_ID_TOAL "TOAL" // Original album/movie/show title
#define ID3V2_FRAME_ID_TOFN "TOFN" // Original filename
#define ID3V2_FRAME_ID_TOLY "TOLY" // Original lyricist(s)/text writer(s)
#define ID3V2_FRAME_ID_TOPE "TOPE" // Original artist(s)/performer(s)
#define ID3V2_FRAME_ID_TOWN "TOWN" // File owner/licensee
#define ID3V2_FRAME_ID_TPE1 "TPE1" // Lead performer(s)/Soloist(s)
#define ID3V2_FRAME_ID_TPE2 "TPE2" // Band/orchestra/accompaniment
#define ID3V2_FRAME_ID_TPE3 "TPE3" // Conductor/performer refinement
#define ID3V2_FRAME_ID_TPE4 "TPE4" // Interpreted, remixed, or otherwise modified by
#define ID3V2_FRAME_ID_TPOS "TPOS" // Part of a set
#define ID3V2_FRAME_ID_TPRO "TPRO" // Produced notice
#define ID3V2_FRAME_ID_TPUB "TPUB" // Publisher
#define ID3V2_FRAME_ID_TRCK "TRCK" // Track number/Position in set
#define ID3V2_FRAME_ID_TRSN "TRSN" // Internet radio station name
#define ID3V2_FRAME_ID_TRSO "TRSO" // Internet radio station owner
#define ID3V2_FRAME_ID_TSOA "TSOA" // Album sort order
#define ID3V2_FRAME_ID_TSOP "TSOP" // Performer sort order
#define ID3V2_FRAME_ID_TSOT "TSOT" // Title sort order
#define ID3V2_FRAME_ID_TSRC "TSRC" // ISRC (international standard recording code)
#define ID3V2_FRAME_ID_TSSE "TSSE" // Software/Hardware and settings used for encoding
#define ID3V2_FRAME_ID_TSST "TSST" // Set subtitle
#define ID3V2_FRAME_ID_TXXX "TXXX" // User defined text information frame
#define ID3V2_FRAME_ID_UFID "UFID" // Unique file identifier
#define ID3V2_FRAME_ID_USER "USER" // Terms of use
#define ID3V2_FRAME_ID_USLT "USLT" // Unsynchronised lyric/text transcription
#define ID3V2_FRAME_ID_WCOM "WCOM" // Commercial information
#define ID3V2_FRAME_ID_WCOP "WCOP" // Copyright/Legal information
#define ID3V2_FRAME_ID_WOAF "WOAF" // Official audio file webpage
#define ID3V2_FRAME_ID_WOAR "WOAR" // Official artist/performer webpage
#define ID3V2_FRAME_ID_WOAS "WOAS" // Official audio source webpage
#define ID3V2_FRAME_ID_WORS "WORS" // Official Internet radio station homepage
#define ID3V2_FRAME_ID_WPAY "WPAY" // Payment
#define ID3V2_FRAME_ID_WPUB "WPUB" // Publishers official webpage
#define ID3V2_FRAME_ID_WXXX "WXXX" // User defined URL link frame

struct id3v2_frame_UFID {
    char *owner;
    uint8_t *id;
};

// T000-TZZZ, excluding TXXX
struct id3v2_frame_text {
    uint8_t encoding;
    char *text;
};

struct id3v2_frame_TXXX {
    uint8_t encoding;
    char *description;
    char *value;
};

// W000-WZZZ, excluding WXXX
struct id3v2_frame_url {
    char *url;
};

struct id3v2_frame_WXXX {
    uint8_t encoding;
    char *description;
    char *url;
};

struct id3v2_frame_MCDI {
    uint8_t *TOC;
};

enum id3v2_timestamp_format {
    ID3V2_TIMESTAMP_FORMAT_MPEG = 1,
    ID3V2_TIMESTAMP_FORMAT_MS
};

#define ID3V2_EVENT_PADDING             0x00
#define ID3V2_EVENT_INITIAL_SILENCE_END 0x01
#define ID3V2_EVENT_INTRO_START         0x02
#define ID3V2_EVENT_MAIN_PART_START     0x03
#define ID3V2_EVENT_OUTRO_START         0x04
#define ID3V2_EVENT_OUTRO_END           0x05
#define ID3V2_EVENT_VERSE_START         0x06
#define ID3V2_EVENT_REFRAIN_START       0x07
#define ID3V2_EVENT_INTERLUDE_START     0x08
#define ID3V2_EVENT_THEME_START         0x09
#define ID3V2_EVENT_VARIATION_START     0x0A
#define ID3V2_EVENT_KEY_CHANGE          0x0B
#define ID3V2_EVENT_TIME_CHANGE         0x0C
#define ID3V2_EVENT_MOMENTARY_NOISE     0x0D
#define ID3V2_EVENT_SUSTAINED_NOISE     0x0E
#define ID3V2_EVENT_SUSTAINED_NOISE_END 0x0F
#define ID3V2_EVENT_INTRO_END           0x10
#define ID3V2_EVENT_MAIN_PART_END       0x11
#define ID3V2_EVENT_VERSE_END           0x12
#define ID3V2_EVENT_REFRAIN_END         0x13
#define ID3V2_EVENT_THEME_END           0x14
#define ID3V2_EVENT_PROFANITY           0x15
#define ID3V2_EVENT_PROFANITY_END       0x16
// 0x17-0xFC reserved
#define ID3V2_EVENT_AUDIO_END           0xFD
#define ID3V2_EVENT_FILE_END            0xFE
#define ID3V2_EVENT_EVENTS_FOLLOW       0xFF

struct id3v2_ETCO_event {
    uint8_t event_type;
    uint32_t timestamp;
};

struct id3v2_frame_ETCO {
    uint8_t timestamp_format;
    struct id3v2_ETCO_event *event_list;
};

struct id3v2_frame_MLLT {
    uint32_t mpeg_frames_between_reference;
    uint24_t bytes_between_reference;
    uint24_t ms_between_reference;
    uint8_t bits_for_bytes_deviation;
    uint8_t bits_for_ms_deviation;
    uint8_t *location_table;
};

struct id3v2_frame_SYTC {
    uint8_t timestamp_format;
    uint8_t *tempo_data;
};

#define ID3V2_LANGUAGE_ID_SIZE 3

struct id3v2_frame_USLT {
    uint8_t encoding;
    char language[ID3V2_LANGUAGE_ID_SIZE];
    char *content_descriptor;
    char *text;
};

enum id3v2_SYLT_text {
    ID3V2_SYLT_TEXT_OTHER,
    ID3V2_SYLT_TEXT_LYRICS,
    ID3V2_SYLT_TEXT_TRANSCRIPTION,
    ID3V2_SYLT_TEXT_MOVEMENT,
    ID3V2_SYLT_TEXT_EVENT,
    ID3V2_SYLT_TEXT_CHORD,
    ID3V2_SYLT_TEXT_TRIVIA,
    ID3V2_SYLT_TEXT_WEBPAGE,
    ID3V2_SYLT_TEXT_IMAGE
};

struct id3v2_frame_SYLT {
    uint8_t encoding;
    char language[ID3V2_LANGUAGE_ID_SIZE];
    uint8_t timestamp_format;
    uint8_t content_type;
    char *content_descriptor;
    uint8_t *synchronized_text;
};

struct id3v2_frame_COMM {
    uint8_t encoding;
    char language[ID3V2_LANGUAGE_ID_SIZE];
    char *content_descriptor;
    char *comment;
};

enum id3v2_RVA2_channel_type {
    ID3V2_RVA2_CHANNEL_OTHER,
    ID3V2_RVA2_CHANNEL_MASTER,
    ID3V2_RVA2_CHANNEL_FRONT_RIGHT,
    ID3V2_RVA2_CHANNEL_FRONT_LEFT,
    ID3V2_RVA2_CHANNEL_BACK_RIGHT,
    ID3V2_RVA2_CHANNEL_BACK_LEFT,
    ID3V2_RVA2_CHANNEL_FRONT_CENTER,
    ID3V2_RVA2_CHANNEL_BACK_CENTER,
    ID3V2_RVA2_CHANNEL_SUBWOOFER
};

struct id3v2_RVA2_adjustment {
    uint8_t channel_type;
    int16_t adjustment;
    uint8_t peak_bits;
    uint8_t *peak_volume;
};

struct id3v2_frame_RVA2 {
    char *identification;
    struct id3v2_RVA2_adjustment *adjustment;
};

enum id3v2_EQU2_interpolation_method {
    ID3V2_EQU2_INTERPOLATION_BAND,
    ID3V2_EQU2_INTERPOLATION_LINEAR
};

struct id3v2_EQU2_adjustment {
    uint16_t frequency;
    int16_t volume_adjustment;
};

struct id3v2_frame_EQU2 {
    uint8_t interpolation_method;
    char *identification;
    struct id3v2_EQU2_adjustment *adjustment_list;
};

#define ID3V2_RVRB_INFINITE_BOUNCES 0xFF

struct id3v2_frame_RVRB {
    uint16_t reverb_left;
    uint16_t reverb_right;
    uint8_t bounces_left;
    uint8_t bounces_right;
    uint8_t feedback_left_to_left;
    uint8_t feedback_left_to_right;
    uint8_t feedback_right_to_right;
    uint8_t feedback_right_to_left;
    uint8_t premix_left_to_right;
    uint8_t premix_right_to_left;
};

enum id3v2_APIC_picture_type {
    ID3V2_APIC_PICTURE_OTHER,
    ID3V2_APIC_PICTURE_FILE_ICON,
    ID3V2_APIC_PICTURE_FRONT_COVER,
    ID3V2_APIC_PICTURE_BACK_COVER,
    ID3V2_APIC_PICTURE_LEAFLET_PAGE,
    ID3V2_APIC_PICTURE_MEDIA,
    ID3V2_APIC_PICTURE_LEAD_ARTIST,
    ID3V2_APIC_PICTURE_ARTIST,
    ID3V2_APIC_PICTURE_CONDUCTOR,
    ID3V2_APIC_PICTURE_ORCHESTRA,
    ID3V2_APIC_PICTURE_COMPOSER,
    ID3V2_APIC_PICTURE_LYRICIST,
    ID3V2_APIC_PICTURE_RECORDING_LOCATION,
    ID3V2_APIC_PICTURE_RECORDING,
    ID3V2_APIC_PICTURE_PERFORMANCE,
    ID3V2_APIC_PICTURE_SCREEN_CAPTURE,
    ID3V2_APIC_PICTURE_BRIGHT_COLORED_FISH, // ?
    ID3V2_APIC_PICTURE_ILLUSTRATION,
    ID3V2_APIC_PICTURE_BAND_LOGOTYPE,
    ID3V2_APIC_PICTURE_PUBLISHER_LOGOTYPE
};

struct id3v2_frame_APIC {
    uint8_t encoding;
    char *mime_type;
    uint8_t picture_type;
    char *description;
    uint8_t *picture;
};

struct id3v2_frame_GEOB {
    uint8_t encoding;
    char *mime_type;
    char *filename;
    char *description;
    uint8_t encapsulated_object;
};

struct id3v2_frame_PCNT {
    uint8_t *play_counter;
};

struct id3v2_frame_POPM {
    char *user_email;
    uint8_t rating;
    uint8_t *play_counter;
};

#define ID3V2_EMBEDDED_INFO_BIT 0x01

struct id3v2_frame_RBUF {
    uint24_t buffer_size;
    uint8_t flags;
    uint32_t tag_offset;
};

struct id3v2_frame_AENC {
    char *owner_id;
    uint16_t preview_start;
    uint16_t preview_length;
    uint8_t *encryption_info;
};

struct id3v2_frame_LINK {
    char frame_id[ID3V2_FRAME_ID_SIZE];
    char *url;
    char *additional_data;
};

#if DEBUG
#define debug(fmt, ...) \
    fprintf(stderr, "%s:%s:%d " fmt "\n", \
        __FILE__, __func__, __LINE__, ##__VA_ARGS__);
#else
#define debug(fmt, ...)
#endif

// Determine whether a value is synchsafe
int is_synchsafe(uint32_t val);

// Convert a synchsafe integer to a normal one
uint32_t from_synchsafe(uint32_t val);

// Convert a normal integer to a synchsafe one
// The input value must be less than 2^28
uint32_t to_synchsafe(uint32_t val);

// Unsynchronize the given data
// If no changes are required, outdata will equal data
// Otherwise, outdata must be freed by the caller
void unsynchronize(uint8_t *data, size_t len, uint8_t **outdata,
        size_t *outlen);

// Resynchronize the given data
// If no changes are required, outdata will equal data
// Otherwise, outdata must be freed by the caller
void resynchronize(uint8_t *data, size_t len, uint8_t **outdata,
        size_t *outlen);

// Verify functions
// Check for compliance with spec and return 1 on success
int verify_id3v2_header(struct id3v2_header *header);
int verify_id3v2_extended_header(struct id3v2_extended_header *extheader);
int verify_id3v2_footer(struct id3v2_footer *footer);

// Find and decode the next ID3v2 tag in the file
// If not included, the extended header and footer will be unchanged
// Caller must free the frame data
// Return 0 if successful, 1 otherwise
int get_id3v2_tag(int fd, struct id3v2_header *header,
        struct id3v2_extended_header *extheader,
        uint8_t **frame_data, size_t *frame_data_len,
        struct id3v2_footer *footer);

// Get the next frame from the frame data
// index should be initialized to zero before the first call, then unmodified
// Returns 0 if successful, 1 otherwise
int get_id3v2_frame(uint8_t *frame_data, size_t frame_data_len, size_t *index,
        struct id3v2_frame_header **header, uint8_t **data);

// Convenience functions for extracting useful information
enum id3v2_restriction_tag_size get_tag_size_restriction(uint8_t flags);
enum id3v2_restriction_text_encoding get_text_encoding_restriction(
        uint8_t flags);
enum id3v2_restriction_text_size get_text_size_restriction(uint8_t flags);
enum id3v2_restriction_image_encoding get_image_encoding_restriction(
        uint8_t flags);
enum id3v2_restriction_image_size get_image_size_restriction(uint8_t flags);

#endif // _ID3V2_H
