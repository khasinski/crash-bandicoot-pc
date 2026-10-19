#ifndef CRASH_ENGINE_STATE_H
#define CRASH_ENGINE_STATE_H

#include "common.h"

/* Partial prefix of the 0x6F4-byte allocation in Card_Alloc.
 * The allocation size is not evidence for the unknown trailing fields. */
typedef struct EngineState {
    s32 word00, word04, word08, word0C, word10, word14, word18;
} EngineState;

typedef struct EngineTileCoords {
    s32 x, y, page;
} EngineTileCoords;

typedef struct EngineTileCell {
    u32 x : 4;
    u32 y : 4;
    s32 tile_x : 9;
    s32 tile_y : 8;
    u32 page : 2;
    u32 reserved_0 : 5;
    u32 clut : 9;
    s32 off_y : 14;
    u32 reserved_1 : 9;
    s32 off_x : 14;
    u32 reserved_2 : 18;
} EngineTileCell;

typedef struct EngineDrawModePacket {
    u32 words[3];
} EngineDrawModePacket;

/* Separate allocation in Title_Init; last observed word is at 0x35D4. */
typedef struct EngineMode25State {
    /* Encoded map reference, derived words, and metadata copied by 31EB4. */
    u32 word00;
    s32 word04, word08, word0C, word10, word14, word18;
    EngineTileCoords coords[528];
    EngineTileCell cells[528];
    u32 word319C;
    u16 cluts[480];
    u16 word3560, word3562, word3564, word3566;
    EngineDrawModePacket draw_modes[4];
    s32 word3598, word359C;
    /* Title_LoadNextState copies the word at g_title_menu_selection into both fields.
     * The instructions establish word width, not pointer semantics. */
    u32 word35A0, word35A4;
    u8 reserved_35a8[0x28];
    s32 word35D0, word35D4;
} EngineMode25State;

/* File I/O cleanup: descriptor address, creation flag, and path to remove
 * when an operation fails. The writer stores its path in the third word. */
typedef struct EngineFileRequest {
    RAM_REF(s32) resource;
    RAM_REF(s32) pending;
    RAM_REF(char) path;
} EngineFileRequest;
typedef EngineFileRequest EngineEventRequest;

typedef struct EngineByteRecord {
    u8 data[6];
} EngineByteRecord;

/* Both save routines process exactly 0x80 bytes. The checksum covers the
 * whole record with its final word cleared; unknown payload bytes stay opaque. */
typedef struct EngineSaveRecord {
    u32 word00, word04, word08, word0C, word10, word14;
    u32 word18, word1C, word20, word24;
    u8 payload[0x54];
    u32 checksum;
} EngineSaveRecord;

/* The writer emits a 0x80-byte file header, followed by three 0x80-byte
 * icon images. Its title is converted to 32 two-byte characters. */
typedef struct EngineCardHeader {
    u8 magic[2], icons, blocks;
    char title[64];
    u8 reserved[0x1C];
    u16 palette[16];
} EngineCardHeader;

/* The same working block is reused for the file header and saved state. */
typedef union EngineCardBlock {
    EngineCardHeader header;
    EngineSaveRecord record;
    u8 bytes[0x80];
} EngineCardBlock;

/* Directory enumeration advances by 0x28. Only the record stride and its
 * use as the filename passed to the BIOS are currently recovered. */
typedef struct EngineDirectoryRecord {
    u8 bytes[0x28];
} EngineDirectoryRecord;

/* Selected file slots advance by 0x4C; +8 indexes the directory records. */
typedef struct EngineDirectorySlot {
    s32 word00;
    u32 word04;
    s32 record_index;
    u8 title[0x40];
} EngineDirectorySlot;

/* These arrays end at the 0x6F4-byte lifecycle allocation's boundary. Keep
 * this view separate from the independent 0x35D8-byte mode-25 allocation. */
typedef struct EngineDirectoryState {
    EngineState prefix;
    s32 empty_scan_count, record_count, record_cursor;
    EngineDirectorySlot slots[15];
    EngineDirectoryRecord records[15];
} EngineDirectoryState;

/* GCC 2.6.3 emits tentative storage in declaration order. The owner defines
 * these globals in retail order instead of declaring them ahead of that list. */
#ifndef CRASH_ENGINE_STATE_STORAGE
extern s32 gp_title_once, gp_title_not_seen;
extern char gp_game_over_zone_name[8], gp_main_menu_zone_name[8], gp_naughty_dog_zone_name[8], gp_load_menu_zone_name[8];
extern char gp_options_menu_zone_name[8], gp_island_1b_zone_name[8], gp_island_1a_zone_name[8], gp_island_2_zone_name[8];
extern char gp_island_3_zone_name[8], gp_scea_universal_zone_name[8], gp_mdat_fmt_name[8], gp_mdat_default_name[8];
extern RAM_REF(EngineState) gp_card;
extern RAM_REF(EngineMode25State) gp_title;
#endif
void Card_OnWriteError(EngineFileRequest *request);
void Card_OnReadGlobalsError(EngineFileRequest *request);
void Title_LoadScreen(s32 *reference, s32 mode);
s32 Title_LoadNextState(void);
void Card_SaveGlobals(EngineSaveRecord *record);
s32 Card_LoadGlobals(EngineSaveRecord *record);
s32 Card_CountFiles(const char *path, EngineDirectoryRecord *entries);

#endif
