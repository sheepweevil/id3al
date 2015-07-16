// Header file with useful definitions for working with ID3v2.
// Copyright 2015 David Gloe.
//
// Protocol information from:
// http://id3.org/Developer%20Information

#ifndef _ID3V2_H
#define _ID3V2_H

#include <stdint.h>

// Header
#define ID3V2_FILE_IDENTIFIER   "ID3"
#define ID3V2_FOOTER_IDENTIFIER "3DI"

#define ID3V2_HEADER_UNSYNCHRONIZATION_BIT 0x80
#define ID3V2_HEADER_EXTENDED_HEADER_BIT   0x40
#define ID3V2_HEADER_EXPERIMENTAL_BIT      0x20
#define ID3V2_HEADER_FOOTER_BIT            0x10

struct id3v2_header {
    char     id[3];    // not null terminated
    uint8_t  version;
    uint8_t  revision;
    uint8_t  flags;
    uint32_t tag_size; // synchsafe
};

// Extended header
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
    ID3V2_RESTRICTION_TAG_SIZE_4KB3   // 32 frames
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

struct id3v2_footer {
    char     id[3];    // not null terminated
    uint8_t  version;
    uint8_t  revision;
    uint8_t  flags;
    uint32_t tag_size; // synchsafe
};

// Frame header
#define ID3V2_FRAME_HEADER_TAG_ALTER_BIT  0x40
#define ID3V2_FRAME_HEADER_FILE_ALTER_BIT 0x20
#define ID3V2_FRAME_HEADER_READ_ONLY_BIT  0x10

#define ID3V2_FRAME_HEADER_GROUPING_BIT          0x40
#define ID3V2_FRAME_HEADER_COMPRESSION_BIT       0x08
#define ID3V2_FRAME_HEADER_ENCRYPTION_BIT        0x04
#define ID3V2_FRAME_HEADER_UNSYNCHRONIZATION_BIT 0x02
#define ID3V2_FRAME_HEADER_DATA_LENGTH_BIT       0x01

struct id3v2_frame_header {
    char     id[4];       // not null terminated
    uint32_t size;        // synchsafe
    uint8_t status_flags;
    uint8_t format_flags;
};

// Encodings
enum id3v2_encoding {
    ID3V2_ENCODING_ISO_8859_1,
    ID3V2_ENCODING_UTF_16,
    ID3V2_ENCODING_UTF_16BE,
    ID3V2_ENCODING_UTF_8
};

// Frames
#define ID3V2_FRAME_AENC "AENC" // Audio encryption
#define ID3V2_FRAME_APIC "APIC" // Attached picture
#define ID3V2_FRAME_ASPI "ASPI" // Audio seek point index
#define ID3V2_FRAME_COMM "COMM" // Comments
#define ID3V2_FRAME_COMR "COMR" // Commercial frame
#define ID3V2_FRAME_ENCR "ENCR" // Encryption method registration
#define ID3V2_FRAME_EQU2 "EQU2" // Equalisation (2)
#define ID3V2_FRAME_ETCO "ETCO" // Event timing codes
#define ID3V2_FRAME_GEOB "GEOB" // General encapsulated object
#define ID3V2_FRAME_GRID "GRID" // Group identification registration
#define ID3V2_FRAME_LINK "LINK" // Linked information
#define ID3V2_FRAME_MCDI "MCDI" // Music CD identifier
#define ID3V2_FRAME_MLLT "MLLT" // MPEG location lookup table
#define ID3V2_FRAME_OWNE "OWNE" // Ownership frame
#define ID3V2_FRAME_PRIV "PRIV" // Private frame
#define ID3V2_FRAME_PCNT "PCNT" // Play counter
#define ID3V2_FRAME_POPM "POPM" // Popularimeter
#define ID3V2_FRAME_POSS "POSS" // Position synchronisation frame
#define ID3V2_FRAME_RBUF "RBUF" // Recommended buffer size
#define ID3V2_FRAME_RVA2 "RVA2" // Relative volume adjustment (2)
#define ID3V2_FRAME_RVRB "RVRB" // Reverb
#define ID3V2_FRAME_SEEK "SEEK" // Seek frame
#define ID3V2_FRAME_SIGN "SIGN" // Signature frame
#define ID3V2_FRAME_SYLT "SYLT" // Synchronised lyric/text
#define ID3V2_FRAME_SYTC "SYTC" // Synchronised tempo codes
#define ID3V2_FRAME_TALB "TALB" // Album/Movie/Show title
#define ID3V2_FRAME_TBPM "TBPM" // BPM (beats per minute)
#define ID3V2_FRAME_TCOM "TCOM" // Composer
#define ID3V2_FRAME_TCON "TCON" // Content type
#define ID3V2_FRAME_TCOP "TCOP" // Copyright message
#define ID3V2_FRAME_TDEN "TDEN" // Encoding time
#define ID3V2_FRAME_TDLY "TDLY" // Playlist delay
#define ID3V2_FRAME_TDOR "TDOR" // Original release time
#define ID3V2_FRAME_TDRC "TDRC" // Recording time
#define ID3V2_FRAME_TDRL "TDRL" // Release time
#define ID3V2_FRAME_TDTG "TDTG" // Tagging time
#define ID3V2_FRAME_TENC "TENC" // Encoded by
#define ID3V2_FRAME_TEXT "TEXT" // Lyricist/Text writer
#define ID3V2_FRAME_TFLT "TFLT" // File type
#define ID3V2_FRAME_TIPL "TIPL" // Involved people list
#define ID3V2_FRAME_TIT1 "TIT1" // Content group description
#define ID3V2_FRAME_TIT2 "TIT2" // Title/songname/content description
#define ID3V2_FRAME_TIT3 "TIT3" //  Subtitle/Description refinement
#define ID3V2_FRAME_TKEY "TKEY" // Initial key
#define ID3V2_FRAME_TLAN "TLAN" // Language(s)
#define ID3V2_FRAME_TLEN "TLEN" // Length
#define ID3V2_FRAME_TMCL "TMCL" // Musician credits list
#define ID3V2_FRAME_TMED "TMED" // Media type
#define ID3V2_FRAME_TMOO "TMOO" // Mood
#define ID3V2_FRAME_TOAL "TOAL" // Original album/movie/show title
#define ID3V2_FRAME_TOFN "TOFN" // Original filename
#define ID3V2_FRAME_TOLY "TOLY" // Original lyricist(s)/text writer(s)
#define ID3V2_FRAME_TOPE "TOPE" // Original artist(s)/performer(s)
#define ID3V2_FRAME_TOWN "TOWN" // File owner/licensee
#define ID3V2_FRAME_TPE1 "TPE1" // Lead performer(s)/Soloist(s)
#define ID3V2_FRAME_TPE2 "TPE2" // Band/orchestra/accompaniment
#define ID3V2_FRAME_TPE3 "TPE3" // Conductor/performer refinement
#define ID3V2_FRAME_TPE4 "TPE4" // Interpreted, remixed, or otherwise modified by
#define ID3V2_FRAME_TPOS "TPOS" // Part of a set
#define ID3V2_FRAME_TPRO "TPRO" // Produced notice
#define ID3V2_FRAME_TPUB "TPUB" // Publisher
#define ID3V2_FRAME_TRCK "TRCK" // Track number/Position in set
#define ID3V2_FRAME_TRSN "TRSN" // Internet radio station name
#define ID3V2_FRAME_TRSO "TRSO" // Internet radio station owner
#define ID3V2_FRAME_TSOA "TSOA" // Album sort order
#define ID3V2_FRAME_TSOP "TSOP" // Performer sort order
#define ID3V2_FRAME_TSOT "TSOT" // Title sort order
#define ID3V2_FRAME_TSRC "TSRC" // ISRC (international standard recording code)
#define ID3V2_FRAME_TSSE "TSSE" // Software/Hardware and settings used for encoding
#define ID3V2_FRAME_TSST "TSST" // Set subtitle
#define ID3V2_FRAME_TXXX "TXXX" // User defined text information frame
#define ID3V2_FRAME_UFID "UFID" // Unique file identifier
#define ID3V2_FRAME_USER "USER" // Terms of use
#define ID3V2_FRAME_USLT "USLT" // Unsynchronised lyric/text transcription
#define ID3V2_FRAME_WCOM "WCOM" // Commercial information
#define ID3V2_FRAME_WCOP "WCOP" // Copyright/Legal information
#define ID3V2_FRAME_WOAF "WOAF" // Official audio file webpage
#define ID3V2_FRAME_WOAR "WOAR" // Official artist/performer webpage
#define ID3V2_FRAME_WOAS "WOAS" // Official audio source webpage
#define ID3V2_FRAME_WORS "WORS" // Official Internet radio station homepage
#define ID3V2_FRAME_WPAY "WPAY" // Payment
#define ID3V2_FRAME_WPUB "WPUB" // Publishers official webpage
#define ID3V2_FRAME_WXXX "WXXX" // User defined URL link frame

struct id3v2_frame_ufid {
    char frame[4];
    char *owner;
    uint8_t *id;
};

struct id3v2_frame_text {
    char frame[4];
    uint8_t encoding;
    char *text;
}

struct id3v2_frame_user_text {
    char frame[4];
    uint8_t encoding;
    char *description;
    char *value;
};

struct id3v2_frame_url {
    char frame[4];
    char *url;
};

struct id3v2_frame_music_cd {
    char frame[4];
    char *TOC;
};

struct id3v2_event_timing {
    char frame[4];
    uint8_t timestamp_format;
};

// Convert a synchsafe integer to a normal one
uint32_t from_synchsafe(uint32_t);

// Convert a normal integer to a synchsafe one
uint32_t to_synchsafe(uint32_t);

// Return true if a synchronization is detected
int synchronized(uint32_t);

// Return true if an unsynchronization is detected
int unsynchronized(uint32_t);

#endif // _ID3V2_H
