#ifndef CRASH_ZONE_H
#define CRASH_ZONE_H

#include "common.h"
#include "resource.h"

/* Link from a camera path to a path of the same or a neighbouring zone.
 * relation bit 0: the link leaves from the path start, bit 1: from its end.
 * goal bit 0: the link enters the other path at its start; bit 2 marks a
 * link that is only followed for 16 frames after a timed event. */
typedef struct ZoneNeighbor {
    u8 relation;
    u8 zone;
    u8 path;
    u8 goal;
} ZoneNeighbor;

typedef struct ZonePoint {
    s16 x, y, z;
    s16 rot_x, rot_y, rot_z;
} ZonePoint;

/* Camera path item of a zone entry. Progress along it is length * 256.
 * A nonzero direction component tells which way the path runs. */
typedef struct ZonePath {
    u32 slst;
    RAM_REF(ResourceEntry) zone;
    u32 neighbor_count;
    ZoneNeighbor neighbors[4];
    u8 entrance;
    u8 exit;
    u16 length;
    s16 cam_mode;
    s16 node_distance;
    s16 cam_zoom;
    u16 reserved_26[3];
    s16 direction_x;
    s16 direction_y;
    s16 direction_z;
    ZonePoint points[1];
} ZonePath;

/* Entries and pages a zone keeps loaded. */
typedef struct ZoneLoadList {
    s32 entry_count;
    s32 page_count;
    u32 entries[8];
    u32 pages[32];
} ZoneLoadList;

typedef struct ZoneColor {
    u8 r, g, b, code;
} ZoneColor;

typedef struct ZoneGfx {
    u8 reserved_00[0x1C];
    u32 flags;
    u32 vram_fill_height;
    u8 midi_seq_state[8];
    ZoneColor color;
} ZoneGfx;

/* First item of a zone entry. */
typedef struct ZoneHeader {
    s32 world_count;
    u8 reserved_004[0x200];
    s32 path_index;
    s32 path_count;
    s32 entity_count;
    s32 neighbor_count;
    u32 neighbors[8];
    ZoneLoadList load;
    u32 display_flags;
    u8 gfx[0x1C];
    u32 flags;
} ZoneHeader;

typedef struct ZoneEntry {
    u8 reserved_00[0x10];
    RAM_REF(void) items[1];
} ZoneEntry;

/* Current zone, a second zone pointer, the camera path and the progress
 * along that path. */
typedef struct LevelPosition {
    RAM_REF(ResourceEntry) zone;
    RAM_REF(ResourceEntry) object_zone;
    RAM_REF(ZonePath) path;
    s32 progress;
} LevelPosition;

extern LevelPosition level_position;

extern RAM_REF(ZonePath) g_pad2_state;
#endif
