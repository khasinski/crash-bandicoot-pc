#include "common.h"
#include "zone.h"

typedef struct LightMat {
    s16 v[3][3];
    s16 b[3];
    s16 c[3][3];
    s16 a[3];
} LightMat;

typedef struct LevelHeader {
    s32 word_0;
    s32 level_id;
} LevelHeader;

#define CRASH_EXT_LEVEL_CHECKPOINT_0
#include "level_externs.h"
#undef CRASH_EXT_LEVEL_CHECKPOINT_0

typedef struct SaveVec {
    s32 x, y, z;
} SaveVec;

typedef struct SaveTransform {
    SaveVec trans;
    SaveVec rot;
    SaveVec scale;
} SaveTransform;

typedef struct SaveObj {
    u8 reserved_00[0x28];
    RAM_REF(ResourceEntry) entry;
    u8 reserved_2c[0x4C];
    RAM_REF(struct SaveObj) collider;
    u8 reserved_7c[4];
    SaveTransform transform;
    s32 vel_x;
    s32 vel_y;
    s32 vel_z;
    s32 target_rot_x;
    s32 target_rot_y;
    u8 reserved_b8[0x10];
    u32 status_a;
    u32 status_b;
    u8 reserved_d0[0x54];
    s32 word_124;
    u8 reserved_128[8];
    s32 word_130;
} SaveObj;

typedef struct LevelSave {
    SaveTransform transform;
    u32 zone_eid;
    s32 path_idx;
    s32 progress;
    s32 level_id;
    s32 flag34;
    u32 flags38[0x130];
    s32 word_4f8;
} LevelSave;

#define CRASH_EXT_LEVEL_CHECKPOINT_1
#include "level_externs.h"
#undef CRASH_EXT_LEVEL_CHECKPOINT_1
extern s32 gp_checkpoint_zone_id;

#define CRASH_EXT_LEVEL_CHECKPOINT_2
#include "level_externs.h"
#undef CRASH_EXT_LEVEL_CHECKPOINT_2

void Level_SaveState(SaveObj *obj, LevelSave *save, s32 flag)
{
    ZoneHeader *header;
    s32 i;

    if ((RAM_AT(ZoneHeader, RAM_AT(ZoneEntry, level_position.zone)->items[0]))->flags & 0x2000) {
        return;
    }
    save->flag34 = flag;
    save->transform = RAM_AT(SaveObj, gp_crash_obj[0])->transform;
    if (!(obj->status_b & 0x200)) {
        save->transform.trans = obj->transform.trans;
    }
    if (g_checkpoint_id_2[0] != -1 && g_checkpoint_id_2[0] != 0) {
        save->transform.trans = g_checkpoint_spawn_trans;
    }
    save->transform.rot.x = 0;
    save->transform.rot.y = 0;
    save->transform.rot.z = 0;
    save->zone_eid = RAM_AT(ResourceEntry, level_position.zone)->id;
    save->progress = level_position.progress;
    save->level_id = RAM_AT(LevelHeader, g_ns_page_pool[0])->level_id;
    for (i = 0; i < (RAM_AT(ZoneHeader, RAM_AT(ZoneEntry, level_position.zone)->items[0]))->path_count; i++) {
        header = RAM_AT(ZoneHeader, RAM_AT(ZoneEntry, level_position.zone)->items[0]);
        if (level_position.path == RAM_AT(ZoneEntry, level_position.zone)->items[i + header->path_index]) {
            save->path_idx = i;
            break;
        }
    }
    save->word_4f8 = g_death_count[0];
    for (i = 0; i < sizeof(g_spawns) / sizeof(u32); i++) {
        save->flags38[i] = g_spawns[i];
    }
}

void Level_Restart(LevelSave *save)
{
    u32 zone_eid;
    ZoneHeader *header, *n_header;
    ZoneEntry *neighbor, *zone;
    SaveObj *player;
    s32 i;

    g_checkpoint_clear_flag[0] = 0;
    if (save->level_id != RAM_AT(LevelHeader, g_ns_page_pool[0])->level_id) {
        gp_pbak_frame_idx = -2;
        gp_checkpoint_zone_id = 1;
        return;
    }
    Gool_SendToColliders(NULL, 0x1300, 0, 0, 0);
    if (level_position.zone != 0) {
        header = RAM_AT(ZoneHeader, RAM_AT(ZoneEntry, level_position.zone)->items[0]);
        level_position.object_zone = 0xFFFFFFFF;
        for (i = 0; i < header->neighbor_count; i++) {
            neighbor = RAM_AT(ZoneEntry, Ns_Lookup(&header->neighbors[i]));
            n_header = RAM_AT(ZoneHeader, neighbor->items[0]);
            if (n_header->display_flags & 1) {
                Gool_ZoneObjectsTerminate(neighbor);
                n_header->display_flags &= ~3;
            }
        }
        Ns_ZoneUnload(&header->load);
    }
    level_position.zone = 0;
    SetGeomScreen(g_view_proj);
    if (gp_checkpoint_zone_id) {
        for (i = 0; i < sizeof(g_spawns) / sizeof(u32); i++) {
            g_spawns[i] = save->flags38[i];
        }
        if (g_checkpoint_id_2[0] != -1) {
            g_spawns[g_checkpoint_id_2[0] >> 8] &= ~2;
            g_spawns[g_checkpoint_id_2[0] >> 8] |= 8;
        }
        for (i = 0; i < 0x130; i++) {
            g_spawns[i] &= ~1;
        }
    }
    zone_eid = save->zone_eid;
    zone = RAM_AT(ZoneEntry, Ns_Open(&zone_eid, 1, 1));
    Level_Update((ResourceEntry *)zone, RAM_AT(ZonePath, zone->items[save->path_idx + RAM_AT(ZoneHeader, zone->items[0])->path_index]), save->progress, !gp_checkpoint_zone_id);
    Ns_Close(&zone_eid, 1);
    if (gp_crash_obj[0] == 0) {
        GOOL_CreateObject((struct GoolObj *)g_gool_obj_list_5, 0, 0, 0, 0, 1);
    }
    RAM_AT(SaveObj, gp_crash_obj[0])->transform = save->transform;
    player = RAM_AT(SaveObj, gp_crash_obj[0]);
    player->entry = level_position.zone;
    player->word_130 = 0;
    player->vel_x = 0;
    player->vel_y = 0;
    player->vel_z = 0;
    player->word_124 = 0;
    player->status_a = player->status_a;
    player->target_rot_y = player->transform.rot.y;
    if (player->collider != 0) {
        RAM_AT(SaveObj, player->collider)->collider = 0;
        player->collider = 0;
    }
    g_draw_stamp[0] = 0;
    g_cam_mat_dirty_flag = 0;
    gp_level_unused_1 = 0;
    g_fade_counter = 0xFFFF;
    if (!gp_checkpoint_zone_id) {
        g_level_restart_fade_cnt[0] += 0x100;
        if (save->flag34) {
            g_level_frame_accum = 0;
        } else {
            g_level_frame_accum += 0x100;
        }
    }
    Level_InitMisc(0);
    if (gp_checkpoint_zone_id) {
        g_death_count[0] = save->word_4f8;
        gp_checkpoint_zone_id = 0;
        if (g_checkpoint_id_2[0] != -1) {
            g_death_count[0] -= 0x100;
        }
    }
}
