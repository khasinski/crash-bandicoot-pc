#include "common.h"
#include "zone.h"

/* One viewpoint considered by Cam_Follow: a path, the progress the camera
 * would take on it and how far that is from the current progress. */
typedef struct CamCandidate {
    ZonePath *path;
    ZonePath *next_path;
    s32 progress;
    s32 exit;
    s32 entrance;
    s32 delta;
    s32 distance;
    s32 flags;
    s32 relation;
    s32 pan_x;
    s32 zoom;
    s32 pan_y;
    s32 found;
} CamCandidate;

typedef struct CamVec {
    s32 x, y, z;
} CamVec;

typedef struct CamObject {
    u8 reserved_00[0x80];
    CamVec trans;
    CamVec rot;
    CamVec scale;
    u8 reserved_a4[0x108 - 0xA4];
    u32 *anim;
    u32 anim_frame;
    u8 reserved_110[0x140 - 0x110];
    s32 cam_zoom;
} CamObject;

typedef struct CamVertex {
    u8 x, y, z;
    u8 nx, ny, nz;
} CamVertex;

typedef struct CamSvtxFrame {
    u8 reserved_00[8];
    s32 x, y, z;
    u8 reserved_14[0x38 - 0x14];
    CamVertex verts[1];
} CamSvtxFrame;

typedef struct CamTgeoHeader {
    u32 reserved_00;
    s32 scale_x;
    s32 scale_y;
    s32 scale_z;
} CamTgeoHeader;

typedef struct CamLevelView {
    u8 reserved_00[4];
    s32 level_id;
} CamLevelView;

/* Camera offsets (z, zoom, x pan, y pan) and their seek directions; only this
 * unit addresses them through $gp. */
s32 gp_cam_speed = -0x12C00;
s32 gp_cam_follow_dist_max = 0x6A400;
s32 gp_cam_follow_vel_x = 0;
s32 gp_cam_follow_vel_y = 0;
s32 gp_cam_follow_dist_min = 0x3E800;
s32 gp_cam_follow_vel_z = 0;

/* .sbss owned by this unit: camera speed along the path and death-cam state. */
asm(".globl gp_cam_follow_state");
s32 gp_cam_follow_state;
asm(".globl gp_cam_death_rot_x");
s32 gp_cam_death_rot_x;
asm(".globl gp_cam_death_rot_y");
s32 gp_cam_death_rot_y;
asm(".globl gp_cam_death_rot_z");
s32 gp_cam_death_rot_z;
asm(".globl gp_cam_death_pan_x");
s32 gp_cam_death_pan_x;
asm(".globl gp_cam_death_pan_y");
s32 gp_cam_death_pan_y;
asm(".globl gp_cam_death_pan_z");
s32 gp_cam_death_pan_z;

/* View of the global state object at g_pause_state (camera fields only). */
typedef struct CamGlobals {
    u8 reserved_000[0xAC0];
    u32 display_flags;      /* 0x800618B0 */
    s32 death_cam;          /* 0x800618B4 */
    u8 reserved_ac8[0xAE0 - 0xAC8];
    s32 game_state;         /* 0x800618D0 */
    u8 reserved_ae4[0xB2C - 0xAE4];
    CamObject *death_obj;   /* 0x8006191C */
    u8 reserved_b30[0xB60 - 0xB30];
    u32 death_vert;         /* 0x80061950 */
    u8 reserved_b64[0xB7C - 0xB64];
    s32 death_zoom_speed;   /* 0x8006196C */
    s32 death_pan_speed;    /* 0x80061970 */
    u8 reserved_b84[0xB9C - 0xB84];
    s32 island_rot;         /* 0x8006198C */
    s32 wordBA0;            /* 0x80061990 */
    s32 island_state;       /* 0x80061994 */
} CamGlobals;

#define CRASH_EXT_CAM_CAMERA_0
#include "cam_externs.h"
#undef CRASH_EXT_CAM_CAMERA_0
/* Scalar names for the zone state also covered by level_position. CamFollow
 * reads it through the struct view, CamUpdate mostly through these. */

#define CRASH_EXT_CAM_CAMERA_1
#include "cam_externs.h"
#undef CRASH_EXT_CAM_CAMERA_1

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define SIGN(x) ((x) > 0 ? 1 : -((x) != 0))

/* Advance the camera along the current path by delta, crossing onto
 * cam->next_path if the step passes the path end. */
void Cam_AdjustProgress(s32 delta, CamCandidate *cam)
{
    s32 progress;

    if (cam->flags & 2) {
        if (cam->next_path != NULL
            && ((progress = delta + g_cam_path_progress) >> 8) >= g_pad2_state->length) {
            progress = (progress - (g_pad2_state->length << 8)) - 1;
            if (!(cam->relation & 2)) {
                progress = -progress;
            }
            Level_Update(cam->next_path->zone, cam->next_path,
                          MIN(cam->progress, progress + cam->entrance), 0);
        } else {
            Level_Update(level_position.zone, g_pad2_state, delta + g_cam_path_progress, 0);
        }
    } else {
        if (cam->next_path != NULL && (progress = g_cam_path_progress) - delta < 0) {
            progress = delta - progress;
            if (!(cam->relation & 2)) {
                progress = -progress;
            }
            progress += cam->entrance;
            Level_Update(cam->next_path->zone, cam->next_path,
                          MAX(cam->progress, progress), 0);
        } else {
            Level_Update(level_position.zone, g_pad2_state, g_cam_path_progress - delta, 0);
        }
    }
}

/* Project trans onto path along its direction vector and fill cam with the
 * resulting progress and distance from the player. */
s32 Cam_GetProgress(CamVec *trans, ZonePath *path, CamCandidate *cam, u8 relation, s32 has_relation)
{
    CamVec diff1, diff2;
    CamVec *zone_rect;
    CamObject *player;
    s32 progress, pt, entrance, exit;

    zone_rect = (CamVec *)((ZoneEntry *)path->zone)->items[1];
    cam->found = 1;
    diff1.x = (trans->x >> 4) - ((path->points[0].x + zone_rect->x) << 4);
    diff1.y = (trans->y >> 4) - ((path->points[0].y + zone_rect->y) << 4);
    diff1.z = (trans->z >> 4) - ((path->points[0].z + zone_rect->z) << 4);
    pt = (path->direction_x * diff1.x + path->direction_y * diff1.y
          + path->direction_z * diff1.z) >> 8;
    progress = pt / path->node_distance;
    exit = 0;
    entrance = 0;
    pt = progress >> 8;
    if (path == g_pad2_state) {
        cam->next_path = NULL;
    } else {
        if (path->direction_x != 0 && diff1.y < -0x3200
            && (((ZoneHeader *)((ZoneEntry *)level_position.zone)->items[0])->flags & 0x40000)) {
            return 0;
        }
        cam->next_path = path;
    }
    if (has_relation) {
        if (relation & 1) {
            entrance = path->entrance;
        }
        if (relation & 2) {
            exit = path->exit;
        }
    }
    if (pt < entrance) {
        if (entrance != 0 || path != g_pad2_state) {
            return 0;
        }
        progress = 0;
        pt = 0;
        cam->found = 0;
    } else if (pt >= path->length - exit) {
        if (exit != 0 || path != g_pad2_state) {
            return 0;
        }
        progress = (path->length << 8) - 1;
        pt = path->length - 1;
        cam->found = 0;
    }
    player = (CamObject *)gp_crash_obj;
    diff2.x = ((player->trans.x + cam->pan_x) >> 8) - zone_rect->x - path->points[pt].x;
    diff2.y = ((player->trans.y + cam->pan_y) >> 8) - zone_rect->y - path->points[pt].y;
    diff2.z = ((player->trans.z + cam->zoom) >> 8) - zone_rect->z - path->points[pt].z;
    if (!cam->found
        && (__builtin_abs(diff2.x) >= 0xC81 || __builtin_abs(diff2.y) >= 0xC81
            || __builtin_abs(diff2.z) >= 0xC81)) {
        return 0;
    }
    cam->path = path;
    cam->progress = progress;
    cam->distance = SquareRoot0(diff2.x * diff2.x + diff2.y * diff2.y + diff2.z * diff2.z);
    return 1;
}

s32 Cam_GetProgress2(CamVec *trans, ZonePath *path, CamCandidate *cam, s32 relation, s32 has_relation)
{
    CamVec diff1, diff2, dir;
    s32 entrance, exit;
    CamVec *zone_rect, *rect;
    CamObject *player;
    s32 i, progress, zoom_offset;
    s32 dot, abs_dot, prev_dot, min_abs_dot;

    entrance = 0;
    exit = 0;
    rect = zone_rect = (CamVec *)((ZoneEntry *)path->zone)->items[1];
    progress = -1;
    cam->found = 1;
    zoom_offset = -cam->zoom << 4;
    for (i = 0; i < path->length; i++) {
        diff1.x = (trans->x >> 8) - (path->points[i].x + rect->x);
        diff1.z = (trans->z >> 8) - (path->points[i].z + rect->z);
        dir.x = Math_Sin(path->points[i].rot_y);
        dir.z = Math_Cos(path->points[i].rot_y);
        dot = zoom_offset - (diff1.x * dir.x + diff1.z * dir.z);
        abs_dot = __builtin_abs(dot);
        if (path->direction_z > 0) {
            if (i == 0 && dot > 0) {
                if (dot > 0x1F400) {
                    cam->found = 0;
                }
                progress = 0;
                break;
            }
            if (i == path->length - 1 && dot < 0) {
                if (dot < -0x1F400) {
                    cam->found = 0;
                }
                progress = (path->length << 8) - 1;
                break;
            }
        } else {
            if (i == 0 && dot < 0) {
                if (dot < -0x1F400) {
                    cam->found = 0;
                }
                progress = 0;
                break;
            }
            if (i == path->length - 1 && dot > 0) {
                if (dot > 0x1F400) {
                    cam->found = 0;
                }
                progress = (path->length << 8) - 1;
                break;
            }
        }
        if (i == 0 || abs_dot < min_abs_dot) {
            if (i != 0 && (dot ^ prev_dot) < 0) {
                progress += (min_abs_dot << 8) / (abs_dot + min_abs_dot);
            } else {
                progress = i << 8;
            }
            prev_dot = dot;
            min_abs_dot = abs_dot;
        }
    }
    if (path == g_pad2_state) {
        cam->next_path = NULL;
    } else {
        cam->next_path = path;
    }
    if (has_relation) {
        if (relation & 1) {
            entrance = path->entrance;
        }
        if (relation & 2) {
            exit = path->exit;
        }
    }
    if ((progress >> 8) < entrance) {
        if (entrance != 0 || path != g_pad2_state) {
            return 0;
        }
        progress = 0;
        cam->found = 0;
    } else if ((progress >> 8) >= path->length - exit) {
        if (exit == 0 && path == g_pad2_state) {
            progress = (path->length << 8) - 1;
            cam->found = 0;
        } else {
            return 0;
        }
    }
    player = (CamObject *)gp_crash_obj;
    diff2.x = ((player->trans.x + cam->pan_x) >> 8) - zone_rect->x - path->points[progress >> 8].x;
    diff2.y = ((player->trans.y + cam->pan_y) >> 8) - zone_rect->y - path->points[progress >> 8].y;
    diff2.z = ((player->trans.z + cam->zoom) >> 8) - zone_rect->z - path->points[progress >> 8].z;
    cam->path = path;
    cam->progress = progress;
    cam->distance = SquareRoot0(diff2.x * diff2.x + diff2.y * diff2.y + diff2.z * diff2.z);
    return 1;
}

/* Path reached through neighbour link i of the current path. */
static __inline__ ZonePath *cam_neighbor_path(s32 i)
{
    ZoneEntry *zone;

    zone = Ns_Lookup(&((ZoneHeader *)level_position.zone->data)
                              ->neighbors[level_position.path->neighbors[i].zone]);
    return zone->items[level_position.path->neighbors[i].path
                       + ((ZoneHeader *)zone->items[0])->path_index];
}

/* Steer the camera: update the offsets from the pad and the zone flags, then
 * find the progress on the current or a linked path that best follows obj
 * and move towards it. */
void Cam_Follow(CamObject *obj, s32 flags)
{
    CamCandidate cams[4];
    CamVec trans;
    CamCandidate *nearest;
    ZonePath *path, *neighbor;
    s32 nearest_distance, y, flags, count;
    s32 new_pan, new_zoom, snap_pan, snap_zoom, total_zoom, distance;
    s32 found, not_same, exit;
    s32 direction;
    u32 header_flags;
    s32 i;

    count = 0;
    nearest_distance = 0x7FFFFFFF;
    flags = 0;
    y = obj->trans.y;
    /* Bit 0: in the first half (or first 50 units) of the path; bit 1: in
     * the second half (or last 50 units). */
    if ((level_position.progress >> 8) < level_position.path->length / 2
        || (level_position.progress >> 8) < 50) {
        flags = 1;
    }
    if ((level_position.progress >> 8) >= level_position.path->length / 2
        || level_position.path->length - (level_position.progress >> 8) < 50) {
        flags |= 2;
    }
    new_zoom = 0;
    new_pan = 0;
    snap_zoom = 1;
    snap_pan = 1;

    header_flags = ((ZoneHeader *)level_position.zone->data)->flags;
    if ((header_flags & 0x80) || gp_cam_follow_dist_max < 0x32000) {
        if (header_flags & 0x8000) {
            gp_cam_follow_vel_x = 1;
        } else {
            gp_cam_follow_vel_x = 0;
        }
    } else if (g_gool_pad_buttons[0] & 0x1000) {
        gp_cam_follow_vel_x = 0;
    } else if (g_gool_pad_buttons[0] & 0x4000) {
        gp_cam_follow_vel_x = 1;
    }
    if (g_gool_pad_buttons[0] & 0x8000) {
        gp_cam_follow_vel_y = 0;
    } else if (g_gool_pad_buttons[0] & 0x2000) {
        gp_cam_follow_vel_y = 1;
    }
    if (gp_cam_follow_vel_x) {
        if (g_ns_page_pool->level_id == 3) {
            gp_cam_speed += 0x3200;
            if (gp_cam_speed > 0x4B000) {
                gp_cam_speed = 0x4B000;
            }
        } else {
            gp_cam_speed += 0x3200;
            if (gp_cam_speed > 0x12C00) {
                gp_cam_speed = 0x12C00;
            }
        }
    } else {
        gp_cam_speed -= 0x3200;
        if (gp_cam_speed < -0x12C00) {
            gp_cam_speed = -0x12C00;
        }
    }
    if (((ZoneHeader *)level_position.zone->data)->flags & 0x4000) {
        if (level_position.path->direction_x) {
            if (gp_cam_follow_vel_y) {
                gp_cam_follow_vel_z = (gp_cam_follow_vel_z + 0x6400
                              > (level_position.path->direction_x > 0 ? 0x4B000 : 0))
                    ? (level_position.path->direction_x > 0 ? 0x4B000 : 0)
                    : gp_cam_follow_vel_z + 0x6400;
            } else {
                gp_cam_follow_vel_z = (gp_cam_follow_vel_z - 0x6400
                              < (level_position.path->direction_x < 0 ? -0x4B000 : 0))
                    ? (level_position.path->direction_x < 0 ? -0x4B000 : 0)
                    : gp_cam_follow_vel_z - 0x6400;
            }
        }
    } else {
        gp_cam_follow_vel_z = 0;
    }

    /* A linked path running up/down sets the pan, one running in depth the
     * zoom; the current path overrides both and is approached gradually. */
    for (i = 0; i < level_position.path->neighbor_count; i++) {
        if (level_position.path->neighbors[i].relation & flags) {
            neighbor = cam_neighbor_path(i);
            if (neighbor->direction_y) {
                new_pan = neighbor->cam_zoom << 8;
            }
            if (neighbor->direction_z) {
                new_zoom = neighbor->cam_zoom << 8;
            }
        }
    }
    if (level_position.path->direction_y) {
        snap_pan = 0;
        new_pan = level_position.path->cam_zoom << 8;
    }
    if (level_position.path->direction_z) {
        snap_zoom = 0;
        new_zoom = level_position.path->cam_zoom << 8;
    }
    if (new_zoom && (level_position.progress >> 8) >= 11
        && (level_position.progress >> 8) < level_position.path->length - 10) {
        if (snap_zoom) {
            gp_cam_follow_dist_max = new_zoom;
        } else {
            gp_cam_follow_dist_max = Gool_Seek(gp_cam_follow_dist_max, new_zoom, 0x1900);
        }
    }
    total_zoom = gp_cam_follow_dist_max + gp_cam_speed + obj->cam_zoom;
    if (new_pan) {
        if (snap_pan) {
            gp_cam_follow_dist_min = new_pan;
        } else {
            gp_cam_follow_dist_min = Gool_Seek(gp_cam_follow_dist_min, new_pan, 0x6400);
        }
    }

    /* Candidate on the current path. */
    direction = level_position.path->direction_z;
    cams[count].pan_x = gp_cam_follow_vel_z;
    cams[count].pan_y = gp_cam_follow_dist_min;
    cams[count].zoom = total_zoom;
    if (direction) {
        trans.x = obj->trans.x;
        trans.y = obj->trans.y;
        trans.z = obj->trans.z;
        found = Cam_GetProgress2(&trans, level_position.path, &cams[count], 0, 0);
    } else {
        trans.x = obj->trans.x + gp_cam_follow_vel_z;
        trans.y = obj->trans.y + gp_cam_follow_dist_min;
        trans.z = total_zoom + obj->trans.z;
        found = Cam_GetProgress(&trans, level_position.path, &cams[count], 0, 0);
    }
    if (found) {
        cams[count].delta = ABS(level_position.progress - cams[count].progress);
        if (level_position.progress > cams[count].progress) {
            cams[count].flags = 1;
        } else {
            cams[count].flags = 2;
        }
        count++;
    }

    /* Candidates on the linked paths, measured through the shared end. */
    for (i = 0; i < level_position.path->neighbor_count; i++) {
        if ((level_position.path->neighbors[i].goal & 4) && (*(u32 *)g_level_zone_flags) - g_cam_goal_stop_flag > 15) {
            continue;
        }
        if (!(level_position.path->neighbors[i].relation & flags)) {
            continue;
        }
        path = cam_neighbor_path(i);
        if (ABS(path->direction_x) == ABS(level_position.path->direction_x)
            && ABS(path->direction_y) == ABS(level_position.path->direction_y)
            && ABS(path->direction_z) == ABS(level_position.path->direction_z)) {
            not_same = 0;
        } else {
            not_same = 1;
        }
        cams[count].pan_x = gp_cam_follow_vel_z;
        cams[count].pan_y = gp_cam_follow_dist_min;
        cams[count].zoom = total_zoom;
        direction = path->direction_z;
        if (direction) {
            trans.x = obj->trans.x;
            trans.y = y;
            trans.z = obj->trans.z;
            found = Cam_GetProgress2(&trans, path, &cams[count],
                                  level_position.path->neighbors[i].goal, not_same);
        } else {
            trans.x = obj->trans.x + gp_cam_follow_vel_z;
            trans.y = y + gp_cam_follow_dist_min;
            trans.z = total_zoom + obj->trans.z;
            found = Cam_GetProgress(&trans, path, &cams[count],
                                  level_position.path->neighbors[i].goal, not_same);
        }
        if (found) {
            if (level_position.path->neighbors[i].relation & 1) {
                exit = 0;
                distance = level_position.progress;
            } else {
                exit = (level_position.path->length << 8) - 1;
                distance = exit - level_position.progress;
            }
            cams[count].exit = exit;
            if (level_position.path->neighbors[i].goal & 1) {
                cams[count].entrance = 0;
                cams[count].delta = distance + 0x100 + cams[count].progress;
                cams[count].relation = 2;
            } else {
                cams[count].entrance = (path->length << 8) - 1;
                cams[count].delta = distance + (((path->length << 8) - 1) - cams[count].progress) + 0x100;
                cams[count].relation = 1;
            }
            cams[count].flags = level_position.path->neighbors[i].relation;
            count++;
        }
    }

    if (count == 0) {
        return;
    }
    /* A path with camera mode 1 wins; otherwise prefer found positions,
     * then the nearest. */
    nearest = &cams[0];
    for (i = 0; i < count; i++) {
        if (cams[i].path->cam_mode == 1) {
            nearest = &cams[i];
            break;
        }
        if (cams[i].found > nearest->found
            || (cams[i].distance < nearest_distance && cams[i].found >= nearest->found)) {
            nearest = &cams[i];
            nearest_distance = cams[i].distance;
        }
    }
    if (nearest->delta == 0) {
        return;
    }
    if (nearest->delta * level_position.path->node_distance <= 30000) {
        /* Close enough: jump there. */
        Level_Update(nearest->path->zone, nearest->path, nearest->progress, 0);
        gp_cam_follow_state = nearest->delta;
    } else {
        if (nearest->delta <= 0x200) {
            gp_cam_follow_state = nearest->delta / 2;
        } else if (nearest->delta >= 0x500) {
            gp_cam_follow_state = MIN(nearest->delta / 2 + 0x100, gp_cam_follow_state + 0x100);
        } else {
            gp_cam_follow_state = 0x200;
        }
        Cam_AdjustProgress(gp_cam_follow_state, nearest);
    }
}

/* Per-frame camera update, chosen by the current path's camera mode:
 * 0 fixed, 1/3 automatic (advance one step a frame, a pad press skips to the
 * next non-automatic path), 5/6 follow (Cam_Follow), 7 island map (step
 * along linked paths towards the stick angle), 8 step one way along the
 * path. The case 1/3 jump to `advance` keeps that update after the loop,
 * where retail has it. */
s32 Cam_Update(void)
{
    u8 unused[0x48];        /* frame space with no remaining use */
    ZoneEntry *zone, *neighbor;
    ZonePath *path, *next_path;
    ZonePath *target;
    s32 target_progress, side, j;
    s32 progress, next_progress, angle, state, skip, index, i;

    if (gp_crash_obj == NULL) {
        return 0;
    }
    if (cam_globals.display_flags & 0x10000) {
        Cam_Death(&cam_globals.death_cam);
        return 0;
    }
    if (!(cam_globals.display_flags & 2)) {
        return 0;
    }
    switch (g_pad2_state->cam_mode) {
    case 5:
    case 6:
        cam_globals.game_state = 0x100;
        Cam_Follow((CamObject *)gp_crash_obj, 0);
        return 1;
    case 0:
        cam_globals.game_state = 0x100;
        return 1;
    case 1:
    case 3:
        skip = 0;
        if ((g_gool_pads & 0xF0) && !(((ZoneHeader *)g_pads->data)->flags & 0x81000)) {
            skip = 1;
        }
        do {
            if (!(((ZoneHeader *)level_position.zone->data)->flags & 0x1000)) {
                cam_globals.game_state = 0;
            }
            if ((g_cam_path_progress >> 8) + 1 < g_pad2_state->length && !skip) {
                goto advance;
            }
            if (g_pad2_state->neighbor_count == 0) {
                return 1;
            }
            zone = Ns_Lookup(&((ZoneHeader *)g_pads->data)
                                      ->neighbors[g_pad2_state->neighbors[0].zone]);
            target = zone->items[g_pad2_state->neighbors[0].path
                               + ((ZoneHeader *)zone->items[0])->path_index];
            if (g_pad2_state->neighbors[0].goal & 1) {
                target_progress = 0;
            } else {
                target_progress = (target->length - 1) << 8;
            }
            Level_Update((ResourceEntry *)zone, target, target_progress, 0);
            if (!(((ZoneHeader *)g_pads->data)->flags & 0x1000)) {
                Level_SaveState(gp_crash_obj, &g_checkpoint_save_state, 1);
            }
            if (!skip) {
                return 1;
            }
        } while (g_pad2_state->cam_mode == 1 || g_pad2_state->cam_mode == 3);
        return 1;
    advance:
        Level_Update(g_pads, g_pad2_state, g_cam_path_progress + 0x100, 0);
        return 1;
    case 8:
        side = cam_globals.island_state & 3;
        index = -1;
        for (j = 0; j < g_pad2_state->neighbor_count; j++) {
            if ((g_pad2_state->neighbors[j].relation & 3) == (side ^ 3)) {
                index = j;
                break;
            }
        }
        if (side & 1) {
            if ((g_cam_path_progress >> 8) + 1 >= g_pad2_state->length) {
                if (index != -1) {
                    zone = Ns_Lookup(&((ZoneHeader *)g_pads->data)
                                              ->neighbors[g_pad2_state->neighbors[index].zone]);
                    target = zone->items[g_pad2_state->neighbors[index].path
                                         + ((ZoneHeader *)zone->items[0])->path_index];
                    if (g_pad2_state->neighbors[index].goal & 1) {
                        target_progress = 0;
                    } else {
                        target_progress = (target->length - 1) << 8;
                    }
                    Level_Update((ResourceEntry *)zone, target, target_progress, 0);
                }
            } else {
                Level_Update(g_pads, g_pad2_state, g_cam_path_progress + 0x100, 0);
            }
        } else {
            if ((g_cam_path_progress >> 8) - 1 < 0) {
                if (index != -1) {
                    zone = Ns_Lookup(&((ZoneHeader *)g_pads->data)
                                              ->neighbors[g_pad2_state->neighbors[index].zone]);
                    target = zone->items[g_pad2_state->neighbors[index].path
                                         + ((ZoneHeader *)zone->items[0])->path_index];
                    if (g_pad2_state->neighbors[index].goal & 1) {
                        target_progress = 0;
                    } else {
                        target_progress = (target->length - 1) << 8;
                    }
                    Level_Update((ResourceEntry *)zone, target, target_progress, 0);
                }
            } else {
                Level_Update(g_pads, g_pad2_state, g_cam_path_progress - 0x100, 0);
            }
        }
        if (g_pad2_state->cam_mode != 8) {
            cam_globals.island_state = 1;
        }
        return 1;
    case 7:
        path = NULL;
        state = cam_globals.island_state;
        switch (state) {
        case 0:
            return 0;
        case 5:
        case 6:
            angle = 0x22;
            break;
        case -1:
            state = 1;
            angle = -(cam_globals.island_rot & 0xFFF);
            break;
        default:
            angle = Gool_AngDiff(g_cam_rot_y, cam_globals.island_rot);
            if (ABS(angle) >= 0x17) {
                angle = SIGN(angle);
            } else {
                angle = 0;
            }
            break;
        }
        next_path = g_pad2_state;
        next_progress = g_cam_path_progress;
        do {
            ZoneHeader *header;
            s32 goal;

            if (angle == 0) {
                break;
            }
            path = next_path;
            progress = next_progress;
            if (cam_globals.island_state == -1) {
                s32 diff = Gool_AngDiff(path->points[progress >> 8].rot_y, ABS(angle));

                if (diff < 0) {
                    diff = -diff;
                }
                if (diff < 0x17) {
                    break;
                }
            }
            next_progress = progress + 0x100;
            if (state & 4) {
                next_progress = progress + 0x400;
            }
            index = -1;
            if (next_progress >= path->length << 8) {
                for (i = 0; i < path->neighbor_count; i++) {
                    if (path->neighbors[i].goal == state) {
                        index = i;
                    } else if (index == -1) {
                        if (next_path->neighbor_count == 1 || (state & 3) == (path->neighbors[i].goal & 3)) {
                            index = i;
                        }
                    }
                }
                if (index == -1) {
                    for (i = 0; i < next_path->neighbor_count; i++) {
                        if (!(next_path->neighbors[i].goal & 4)) {
                            index = i;
                            break;
                        }
                    }
                }
                header = (ZoneHeader *)next_path->zone->data;
                neighbor = Ns_Lookup(&header->neighbors[next_path->neighbors[index].zone]);
                goal = next_path->neighbors[index].goal;
                next_path = neighbor->items[next_path->neighbors[index].path + ((ZoneHeader *)neighbor->items[0])->path_index];
                if (!(goal & 1)) {
                    next_progress = (next_path->length - 1) << 8;
                } else {
                    next_progress = 0;
                }
                if (state & 4) {
                    path = next_path;
                    progress = next_progress;
                    break;
                }
            } else if (state & 4) {
                progress = next_progress;
                break;
            }
            if (angle > 0) {
                path = next_path;
                progress = next_progress;
                break;
            }
        } while (next_path != g_pad2_state || (next_progress >> 8) != (g_cam_path_progress >> 8));
        if (cam_globals.island_state == -1) {
            cam_globals.island_state = state;
        }
        if (path != NULL && (path != g_pad2_state || progress != g_cam_path_progress)) {
            Level_Update(path->zone, path, progress, 0);
        }
        return 1;
    default:
        return 1;
    }
}

/* Death-camera tracking: focus on a vertex of death_obj's current animation
 * frame and orbit/zoom around it. */
s32 Cam_Death(s32 *arg0)
{
    CamVec v1, v2, v3, v4, v5, v6;
    CamObject *obj;
    CamSvtxFrame *frame;
    ZoneEntry *tgeo;
    CamVertex *vert;
    CamTgeoHeader *header;
    CamVec *p;
    CamVec *cam_pos;
    s32 dx, dy, dz, dist_xz, angle;
    s32 x, y, z;

    obj = cam_globals.death_obj;
    frame = (CamSvtxFrame *)Ns_Lookup(&obj->anim[1])->items[obj->anim_frame >> 8];
    tgeo = Ns_Lookup(&((u32 *)Ns_Lookup(&obj->anim[1])->items[obj->anim_frame >> 8])[1]);
    vert = &frame->verts[cam_globals.death_vert >> 8];
    header = (CamTgeoHeader *)tgeo->items[0];
    v2.x = (frame->x - 0x80 + vert->x) << 10;
    v2.y = (frame->y - 0x80 + vert->y) << 10;
    v2.z = (frame->z - 0x80 + vert->z) << 10;
    v3.x = (obj->scale.x * header->scale_x) >> 12;
    v3.y = (obj->scale.y * header->scale_y) >> 12;
    v3.z = (obj->scale.z * header->scale_z) >> 12;
    p = &v1;
    Gool_Transform(&v2, &obj->trans, &obj->rot, &v3, p);
    dx = (g_cam_trans_x - p->x) >> 8;
    dz = (g_cam_trans_z - p->z) >> 8;
    dy = (g_cam_trans_y - p->y) >> 8;
    dist_xz = SquareRoot0(dx * dx + dz * dz);
    angle = ratan2(-dx, -dz) + 0x800;
    if (*arg0 < 9) {
        if (*arg0 == 0) {
            gp_cam_death_rot_y = Gool_AngDiff(g_cam_rot_y, angle) / 9;
        }
        gp_cam_death_pan_z = dist_xz << 8;
        gp_cam_death_pan_x = 0;
        gp_cam_death_pan_y = angle;
        gp_cam_death_rot_x = 0x16;
        g_cam_rot_y += gp_cam_death_rot_y;
        (*arg0)++;
    }
    if (Gool_AngDiff(g_cam_rot_x, -ratan2(dy, dist_xz)) >= 0x72) {
        gp_cam_death_rot_z = cam_globals.death_pan_speed;
    } else {
        gp_cam_death_rot_z = cam_globals.death_pan_speed / 2;
    }
    g_cam_rot_x = Gool_ObjectRotate(g_cam_rot_x, -ratan2(dy, dist_xz), gp_cam_death_rot_z, 0);
    cam_pos = (CamVec *)&g_cam_trans_x;
    if (cam_globals.display_flags & 0x40000) {
        gp_cam_death_pan_x += gp_cam_death_rot_x;
        g_cam_rot_y += gp_cam_death_rot_x;
    }
    g_cam_trans_y = Gool_Seek(g_cam_trans_y, p->y + 0x1D4C0, 0x19000);
    gp_cam_death_pan_z = Gool_Seek(dist_xz << 8, 0x2AB98, cam_globals.death_zoom_speed);
    x = p->x;
    y = g_cam_trans_y;
    z = p->z;
    v4.x = 0;
    v4.y = 0;
    v4.z = gp_cam_death_pan_z;
    v5.x = x;
    v5.y = y;
    v5.z = z;
    v6.x = 0;
    v6.y = gp_cam_death_pan_y + gp_cam_death_pan_x;
    v6.z = 0;
    Gool_Transform(&v4, &v5, &v6, NULL, cam_pos);
    return -0xFF;
}
