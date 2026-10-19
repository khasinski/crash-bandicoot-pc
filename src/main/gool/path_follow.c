#include "common.h"

typedef struct GoolVec3 {
    s32 x, y, z;
} GoolVec3;

typedef struct GoolVectors {
    s32 trans[3];
    s32 rot[3];
    s32 scale[3];
} GoolVectors;

typedef struct GoolCodeHeader {
    s32 word_00;
    s32 word_04;
    s32 word_08;
    s32 word_0c;
    s32 word_10;
} GoolCodeHeader;

typedef struct GoolExecutable {
    u8 reserved_00[0x10];
    RAM_REF(GoolCodeHeader) header;
    RAM_REF(u8) code;
    RAM_REF(u8) data;
    RAM_REF(u16) states;
    RAM_REF(void) word_20;
    RAM_REF(u32) anims;
} GoolExecutable;

typedef struct GoolDefaults {
    u8 bytes[0x2A];
    u16 intensity_r, intensity_g, intensity_b;
} GoolDefaults;

typedef struct ResourceEntry {
    u8 reserved_00[0x10];
    RAM_REF(u8) data;
    RAM_REF(void) word_14;
} ResourceEntry;

typedef struct GoolObj {
    s32 kind;
    s32 word_04;
    s32 bound[6];
    RAM_REF(GoolExecutable) exec;
    s32 word_24;
    RAM_REF(ResourceEntry) zone;
    s32 word_2c;
    GoolDefaults defaults;
    RAM_REF(struct GoolObj) self;
    RAM_REF(struct GoolObj) parent;
    RAM_REF(struct GoolObj) next;
    RAM_REF(struct GoolObj) children;
    s32 creator;
    RAM_REF(void) player;
    RAM_REF(struct GoolObj) collider;
    s32 interrupter;
    GoolVectors vectors;
    s32 vel_x;
    s32 vel_y;
    s32 vel_z;
    s32 target_rot_x;
    s32 target_rot_y;
    s32 target_rot_z;
    s32 rot_vel_x;
    s32 rot_vel_y;
    s32 rot_vel_z;
    u32 status_a;
    u32 status_b;
    u32 status_c;
    s32 subtype;
    s32 pid_flags;
    RAM_REF(u32) sp;
    RAM_REF(u32) pc;
    RAM_REF(u32) fp;
    RAM_REF(u32) tp;
    RAM_REF(u32) ep;
    s32 result;
    s32 misc_flag;
    s32 arg0;
    s32 arg1;
    s32 arg2;
    s32 arg3;
    RAM_REF(void) anim_seq;
    s32 anim_frame;
    RAM_REF(void) entity;
    s32 path_progress;
    s32 path_length;
    s32 floor_y;
    u32 state_flags;
    s32 speed;
    s32 invincibility_state;
    s32 invincibility_time;
    s32 floor_time;
    s32 floor_vel_y;
    s32 cam_zoom;
    s32 event;
    s32 cam_rot_x;
    s32 ang_vel_x;
    s32 head_turn;
    s32 voice_id;
    s32 ack_proj;
    s32 hotspot_size;
    s32 node_id;
} GoolObj;

typedef struct SVec3 {
    s16 x, y, z;
} SVec3;

typedef struct ZoneEntryView {
    u32 word00;
    u32 id;
    s32 kind;
    s32 word0C;
    RAM_REF(void) items[2];
} ZoneEntryView;

typedef struct ZoneEntity {
    RAM_REF(ZoneEntryView) zone;
    u16 flags;
    u8 group;
    u8 reserved_07;
    u16 id;
    u16 path_length;
    s16 rot_x;
    s16 rot_y;
    s16 rot_z;
    u8 type;
    u8 subtype;
    SVec3 points[1];
} ZoneEntity;

typedef struct Unk4Bytes {
    u8 bytes[4];
} Unk4Bytes;

#define CRASH_EXT_GOOL_PATH_FOLLOW
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_PATH_FOLLOW

#define ABS(x) ((x) < 0 ? -(x) : (x))

s32 Gool_ObjectOrientOnPath(GoolObj *obj, s32 progress, s32 *trans_arr) {
    GoolVec3 *trans = (GoolVec3 *)trans_arr;
    GoolVec3 next_pt;
    GoolVec3 diff;
    GoolVec3 proj;
    ZoneEntity *entity;
    GoolVec3 *origin;
    s32 shift;
    s32 idx;
    s32 frac;
    SVec3 *pt;
    s32 dist;
    s32 dot;
    u32 old_status_a;

    if (obj == NULL) {
        return -0xE;
    }
    entity = RAM_AT(ZoneEntity, obj->entity);
    if (entity == NULL) {
        return -0xE;
    }
    switch (RAM_AT(ZoneEntryView, entity->zone)->kind) {
    case 7:
        origin = RAM_AT(GoolVec3, RAM_AT(ZoneEntryView, entity->zone)->items[1]);
        shift = 2;
        break;
    case 17:
        origin = &g_path_orient_zero_vec;
        shift = 0;
        break;
    }
    if ((idx = ABS(progress) >> 8) == entity->path_length - 1 && idx != 0) {
        frac = (progress & 0xFF) + 0x100;
        idx = entity->path_length - 2;
    } else {
        frac = progress & 0xFF;
    }
    pt = &entity->points[idx];
    trans->x = ((pt->x << shift) + origin->x) << 8;
    trans->y = ((pt->y << shift) + origin->y) << 8;
    trans->z = ((pt->z << shift) + origin->z) << 8;
    if (entity->path_length == 1) {
        return 1;
    }
    pt++;
    next_pt.x = ((pt->x << shift) + origin->x) << 8;
    next_pt.y = ((pt->y << shift) + origin->y) << 8;
    next_pt.z = ((pt->z << shift) + origin->z) << 8;
    diff.x = next_pt.x - trans->x;
    diff.y = next_pt.y - trans->y;
    diff.z = next_pt.z - trans->z;
    if (obj->status_b & 0x8000) {
        dot = (next_pt.z - trans->z) >> 8;
        dist = SquareRoot0(
            ((next_pt.x - trans->x) >> 8) * ((next_pt.x - trans->x) >> 8) +
            dot * dot);
        if (dist != 0) {
            s32 abs_dx;
            s32 abs_dz;
            s32 temp;

            temp = ((obj->vectors.trans[0] - trans->x) >> 4) * ((next_pt.x - trans->x) >> 4) +
                   ((obj->vectors.trans[2] - trans->z) >> 4) * ((next_pt.z - trans->z) >> 4);
            dot = temp / dist;
            frac = dot / dist;
            if (frac >= 0x100 && (progress >> 8) < entity->path_length - 1) {
                return Gool_ObjectOrientOnPath(obj, (idx + 1) << 8, (s32 *)trans);
            }
            proj.x = (((dot >> 4) * ((next_pt.x - trans->x) >> 4)) / dist) + trans->x;
            proj.z = (((dot >> 4) * ((next_pt.z - trans->z) >> 4)) / dist) + trans->z;
            abs_dx = ABS(proj.x - obj->vectors.trans[0]);
            abs_dz = ABS(proj.z - obj->vectors.trans[2]);
            if (abs_dz < abs_dx) {
                obj->rot_vel_y = abs_dx + abs_dz / 2;
            } else {
                obj->rot_vel_y = abs_dz + abs_dx / 2;
            }
            if (obj->rot_vel_y > obj->hotspot_size || (frac < 0 && idx == 0) || frac >= 0x100) {
                obj->status_a |= 0x200;
            }
            if ((next_pt.z - trans->z) * (obj->vectors.trans[0] >> 8) -
                (next_pt.x - trans->x) * (obj->vectors.trans[2] >> 8) -
                (trans->x >> 8) * next_pt.z +
                (trans->z >> 8) * next_pt.x < 0) {
                obj->rot_vel_y = -obj->rot_vel_y;
            }
        } else {
            return -0xE;
        }
    }
    old_status_a = obj->status_a;
    if (obj->path_progress < 0 || (progress >> 8) >= entity->path_length - 1) {
        obj->status_a = old_status_a | 4;
    } else {
        obj->status_a = old_status_a & ~4;
    }
    if ((obj->status_a & 4) != (old_status_a & 4) && !(obj->status_a & 0x10)) {
        obj->status_a |= 0x10;
    } else {
        obj->status_a &= ~0x10;
    }
    if (obj->status_b & 4) {
        if ((obj->status_b & 2) && (obj->status_a & 4)) {
            obj->target_rot_y = ratan2(-diff.x, -diff.z);
        } else {
            obj->target_rot_y = ratan2(diff.x, diff.z);
        }
        if (obj->status_b & 0x800) {
            s32 abs_dx = ABS(diff.x);
            s32 abs_dz = ABS(diff.z);
            obj->vectors.rot[2] = ratan2(diff.x, diff.z);
            obj->target_rot_z = -ratan2(diff.y, abs_dz < abs_dx ? abs_dx + abs_dz / 2 : abs_dz + abs_dx / 2);
        }
    } else if (obj->status_b & 0x800) {
        s32 abs_dx = ABS(diff.x);
        s32 abs_dz = ABS(diff.z);
        obj->target_rot_z = ratan2(diff.y, abs_dz < abs_dx ? abs_dx + abs_dz / 2 : abs_dz + abs_dx / 2);
        obj->vectors.rot[2] = ratan2(-diff.x, -diff.z);
    }
    next_pt.x = ((next_pt.x - trans->x) * frac) >> 8;
    next_pt.y = ((next_pt.y - trans->y) * frac) >> 8;
    next_pt.z = ((next_pt.z - trans->z) * frac) >> 8;
    trans->x += next_pt.x;
    trans->y += next_pt.y;
    trans->z += next_pt.z;
    return entity->path_length;
}
