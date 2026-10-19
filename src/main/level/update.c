#include "common.h"
#include "zone.h"
#include "ns.h"

typedef struct LvlVec {
    s32 x, y, z;
} LvlVec;

/* Camera vectors at 0x80057864: position, rotation, and copies of the
 * rotation from before and after the latest path update. */
typedef struct LvlCamera {
    LvlVec trans;
    LvlVec rot;
    u8 reserved_18[0xD0 - 0x18];
    LvlVec rot_after;
    LvlVec rot_before;
} LvlCamera;

typedef struct LvlColorState {
    ZoneColor prev;
    ZoneColor cur;
    ZoneColor target;
    u32 start_tick;
} LvlColorState;

#define CRASH_EXT_LEVEL_UPDATE_0
#include "level_externs.h"
#undef CRASH_EXT_LEVEL_UPDATE_0

/* SLST polygon id lists: current, and the two buffers swapped per step. */
#define CRASH_EXT_LEVEL_UPDATE_1
#include "level_externs.h"
#undef CRASH_EXT_LEVEL_UPDATE_1

/* Whether one of the header's neighbour links resolves to entry. */
static __inline__ s32 zone_has_neighbor(ZoneHeader *header, ZoneEntry *entry)
{
    s32 i;

    for (i = 0; i < header->neighbor_count; i++) {
        if (Ns_Lookup(&header->neighbors[i]) == (s32)Ram_Addr(entry)) {
            return 1;
        }
    }
    return 0;
}

typedef struct ZoneEntityHeader {
    u8 reserved_00[6];
    u8 group;
} ZoneEntityHeader;

/* Spawn type-3 entities of zone when its pending-spawn display flag is set. */
void Level_SpawnObjects(ZoneEntry *zone)
{
    ZoneHeader *header, *cur;
    ZoneEntityHeader *entity;
    s32 i;

    header = RAM_AT(ZoneHeader, zone->items[0]);
    if (header->display_flags & 2) {
        for (i = 0; i < header->entity_count; i++) {
            cur = RAM_AT(ZoneHeader, zone->items[0]);
            entity = RAM_AT(ZoneEntityHeader, zone->items[i + cur->path_index + cur->path_count]);
            if (entity->group == 3) {
                Gool_ObjectSpawn((struct ZoneEntryView *)zone, i);
            }
        }
        header->display_flags &= ~2;
    }
}

/* Spawn pending type-3 entities across all neighbours of the current zone. */
void Level_SpawnObjects2(void)
{
    ZoneHeader *header;
    ZoneEntry *neighbor;
    s32 i;

    if (level_position.zone == 0) {
        return;
    }
    header = RAM_AT(ZoneHeader, RAM_AT(ZoneEntry, level_position.zone)->items[0]);
    for (i = 0; i < header->neighbor_count; i++) {
        neighbor = RAM_AT(ZoneEntry, Ns_Lookup(&header->neighbors[i]));
        Level_SpawnObjects(neighbor);
    }
}

/* LevelUpdate: move the camera to progress along path in zone. Updates the
 * visible polygon lists (SLST) point by point, and on a zone change unloads
 * the old zone's neighbours, loads the new zone's entries and pages and
 * marks its neighbours. Clears the NS update-pending word at the end. */
void Level_Update(ResourceEntry *zone, ZonePath *path, s32 progress, u32 flags)
{
    ZoneHeader *header, *cur_header, *n_header, *neighbor_header;
    ZoneEntry *slst, *neighbor;
    u32 tmp;
    s32 last, pt, next, steps, i, flag;

    if (path == NULL || zone == NULL) {
        goto done;
    }
    last = (path->length << 8) - 1;
    if (progress < last) {
        last = progress;
        if (last < 0) {
            last = 0;
        }
    }
    progress = last;
    if (RAM_AT(ZoneHeader, ((ZoneEntry *)zone)->items[0])->world_count) {
        if (level_position.zone != Ram_Addr(zone) || Ram_Addr(path) != level_position.path
            || (level_position.progress >> 8) != (progress >> 8)) {
            slst = RAM_AT(ZoneEntry, Ns_Open(&path->slst, 1, 1));
            pt = progress >> 8;
            steps = (progress - level_position.progress) >> 8;
            /* pt + 1 goes through a variable: written inline, fold turns
             * it into (length - 1) - pt, which retail does not have. */
            if (Ram_Addr(path) != level_position.path || __builtin_abs(steps) > pt
                || __builtin_abs(steps) > path->length - (next = pt + 1)) {
                tmp = gp_slst_poly_list_b;
                gp_slst_poly_list_b = gp_slst_poly_list_a;
                gp_slst_poly_list_a = tmp;
                if (pt < path->length / 2) {
                    gp_level_entry = Ram_Addr(Slst_Update(RAM_AT(struct Stream, slst->items[0]), RAM_AT(struct Stream, gp_level_entry), RAM_AT(struct Stream, gp_slst_poly_list_a), 1));
                    pt = 0;
                } else {
                    gp_level_entry = Ram_Addr(Slst_Update(RAM_AT(struct Stream, slst->items[path->length]), RAM_AT(struct Stream, gp_level_entry), RAM_AT(struct Stream, gp_slst_poly_list_a), 0));
                    pt = path->length - 1;
                }
            } else {
                pt = level_position.progress >> 8;
            }
            steps = (progress >> 8) - pt;
            while (steps != 0) {
                tmp = gp_slst_poly_list_b;
                gp_slst_poly_list_b = gp_slst_poly_list_a;
                gp_slst_poly_list_a = tmp;
                if (steps > 0) {
                    steps--;
                    gp_level_entry = Ram_Addr(Slst_Update(RAM_AT(struct Stream, slst->items[++pt]), RAM_AT(struct Stream, gp_level_entry), RAM_AT(struct Stream, gp_slst_poly_list_a), 1));
                } else {
                    gp_level_entry = Ram_Addr(Slst_Update(RAM_AT(struct Stream, slst->items[pt--]), RAM_AT(struct Stream, gp_level_entry), RAM_AT(struct Stream, gp_slst_poly_list_a), 0));
                    steps++;
                }
            }
            Ns_Close(&path->slst, 1);
        } else {
            if (level_position.progress == progress) {
                goto done;
            }
            level_position.progress = progress;
            goto camera;
        }
    }
    if (level_position.zone != Ram_Addr(zone)) {
        flag = 1;
        cur_header = NULL;
        header = RAM_AT(ZoneHeader, ((ZoneEntry *)zone)->items[0]);
        g_level_first_spawn_flag[0] = 0;
        if (g_game_substate == 0x600) {
            if (flags & 2) {
                flag = 1;
            } else {
                flag = 0;
            }
        }
        if (level_position.zone != 0) {
            cur_header = RAM_AT(ZoneHeader, RAM_AT(ZoneEntry, level_position.zone)->items[0]);
            flag = flags & 2;
            level_position.object_zone = Ram_Addr(zone);
            for (i = 0; i < cur_header->neighbor_count; i++) {
                neighbor = RAM_AT(ZoneEntry, Ns_Lookup(&cur_header->neighbors[i]));
                if (zone_has_neighbor(RAM_AT(ZoneHeader, ((ZoneEntry *)zone)->items[0]), neighbor)) {
                    continue;
                }
                n_header = RAM_AT(ZoneHeader, neighbor->items[0]);
                if (n_header->display_flags & 1) {
                    Gool_ZoneObjectsTerminate(neighbor);
                    n_header->display_flags &= ~3;
                }
            }
        }
        if (flags & 1) {
            for (i = 0; i < 0x130; i++) {
                g_spawns[i] &= ~2;
                g_spawns[i] &= ~4;
            }
        }
        level_position.zone = Ram_Addr(zone);
        level_position.path = Ram_Addr(path);
        level_position.progress = progress;
        Ns_ZoneUnload(cur_header != NULL ? &cur_header->load : NULL);
        for (i = 0; i < header->load.entry_count; i++) {
            Ns_Open(&header->load.entries[i], 0, 1);
        }
        for (i = 0; i < header->load.page_count; i++) {
            Ns_PageOpen(header->load.pages[i], 0, 1, 0x6396347F);
        }
        if (flag) {
            Ns_Update2();
        }
        for (i = 0; i < header->neighbor_count; i++) {
            neighbor = RAM_AT(ZoneEntry, Ns_Lookup(&header->neighbors[i]));
            neighbor_header = RAM_AT(ZoneHeader, neighbor->items[0]);
            if (!(neighbor_header->display_flags & 1)) {
                g_doctor_cortex_obj[0] = 0;
                g_mdat_title_str_ptr[0] = 0;
                g_spu_reverb_addr[0] = 0x19000;
                neighbor_header->display_flags |= 3;
            }
            if (flag) {
                RAM_AT(ZoneHeader, neighbor->items[0])->display_flags |= 4;
            } else {
                RAM_AT(ZoneHeader, neighbor->items[0])->display_flags &= ~4;
            }
        }
        Level_UpdateMisc(header->gfx, flags);
    } else {
        level_position.zone = Ram_Addr(zone);
        level_position.path = Ram_Addr(path);
        level_position.progress = progress;
    }
camera:
    lvl_camera.rot_before = lvl_camera.rot;
    Zone_PathProgressToLoc(RAM_AT(ZonePath, level_position.path), -level_position.progress, (struct CamSample *)&lvl_camera.trans);
    lvl_camera.rot_after = lvl_camera.rot;
done:
    g_ns.wordA98 = 0;
}

/* Update the zone colour transition state and graphics parameters. */
void Level_UpdateMisc(void *arg0, u32 flags)
{
    ZoneGfx *gfx = (ZoneGfx *)arg0;

    lvl_color.start_tick = engine_counter[0];
    lvl_color.prev = lvl_color.cur;
    lvl_color.target = gfx->color;
    g_game_state[0] = gfx->flags;
    if (!(flags & 4)) {
        Midi_SetStateStopped();
    }
}
