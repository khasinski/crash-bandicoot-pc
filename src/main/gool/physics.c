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

typedef struct CamTransformState {
    u8 mat7E4[0x60];
    u8 mat844[0x20];
    u8 vec864[0x24];
    u8 vec888[0x48];
} CamTransformState;

#define CRASH_EXT_GOOL_PHYSICS_0
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_PHYSICS_0

typedef struct GoolColor {
    u16 r, g, b;
} GoolColor;

#define CRASH_EXT_GOOL_PHYSICS_1
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_PHYSICS_1

static __inline__ void Gool_ObjectColorByZone(GoolObj *object, s32 mode) {
    ResourceEntry *entry;
    GoolDefaults *defaults;

    entry = RAM_AT(ResourceEntry, object->zone);
    if (entry == NULL) {
        entry = RAM_AT(ResourceEntry, g_pads[0]);
    }
    if (Ram_Addr(object) == gp_crash_obj) {
        defaults = RAM_AT(GoolDefaults, entry->data + 0x348);
    } else {
        defaults = RAM_AT(GoolDefaults, entry->data + 0x318);
    }
    switch (mode) {
    case 0:
        object->defaults = *defaults;
        return;
    case 1:
        object->defaults.intensity_r = defaults->intensity_r;
        object->defaults.intensity_g = defaults->intensity_g;
        object->defaults.intensity_b = defaults->intensity_b;
        return;
    }
}

s32 Gool_ObjectColors(GoolObj *obj) {
    s32 sp18;
    s32 rem;
    GoolDefaults *defaults = &obj->defaults;

    switch (obj->invincibility_state) {
    case 6:
        if (g_level_zone_flags[0] - obj->invincibility_time >= 0x10U) {
            obj->invincibility_state = 0;
            obj->status_b |= 0x80;
        }
        break;
    case 7:
        if (g_level_zone_flags[0] - obj->invincibility_time >= 0x10U || (obj->status_a & 1)) {
            obj->invincibility_state = 0;
            obj->status_b |= 0x80;
        }
        break;
    case 4:
        if (g_level_zone_flags[0] - obj->invincibility_time >= 0x3DU) {
            obj->invincibility_state = 0;
            Gool_ObjectColorByZone(obj, 1);
        }
        /* fallthrough */
    case 3:
        if (obj->invincibility_state == 3 && g_level_zone_flags[0] - obj->invincibility_time >= 0x1C4U) {
            obj->invincibility_state = 0;
            Gool_ObjectColorByZone(obj, 1);
        } else if (obj->invincibility_state == 4) {
            if (obj->collider != 0 && RAM_AT(GoolCodeHeader, RAM_AT(GoolExecutable, RAM_AT(GoolObj, obj->collider)->exec)->header)->word_04 == 0x300) {
                sp18 = 0;
                Gool_ExecStateCode((struct GoolObjView *)obj, RAM_AT(struct GoolObjView, obj->collider), 0xA00, 1, &sp18);
            }
        }
        /* fallthrough */
    case 5:
        if (obj->invincibility_state == 5 && g_level_zone_flags[0] - obj->invincibility_time >= 0x25BU) {
            obj->invincibility_state = 0;
            Gool_ObjectColorByZone(obj, 1);
        }
        /* fallthrough */
    case 2:
        rem = (g_draw_stamp[0] % 4) << 8;
        if (rem >= 0x100) {
            defaults->intensity_r = 0x47F - rem;
        } else {
            defaults->intensity_r = rem + 0x7F;
        }
        defaults->intensity_g = defaults->intensity_b = defaults->intensity_r;
        break;
    case 1:
        break;
    default:
        if (Ram_Addr(obj) == gp_crash_obj && (obj->status_b & 0x4000000)) {
            typedef struct ZoneColorView {
                u8 reserved_00[0x372];
                GoolColor color372;
            } ZoneColorView;
            *(GoolColor *)&obj->defaults.intensity_r = RAM_AT(ZoneColorView, RAM_AT(ResourceEntry, obj->zone)->data)->color372;
        }
        break;
    }
    return -0xFF;
}

typedef struct GoolBound {
    s32 x1;
    s32 y1;
    s32 z1;
    s32 x2;
    s32 y2;
    s32 z2;
} GoolBound;

typedef struct GoolBoundEntry {
    GoolBound bound;
    RAM_REF(GoolObj) obj;
} GoolBoundEntry;

typedef struct GoolBoundsGlobals {
    GoolBoundEntry bounds[96];
    s32 count;
} GoolBoundsGlobals;

#define CRASH_EXT_GOOL_PHYSICS_2
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_PHYSICS_2

void Gool_ObjectControlDir(GoolObj *obj) {
    s32 trans[3];
    GoolBound player_bound;
    GoolBoundEntry *entry;
    u32 *res;
    s32 *model;
    s32 angle;
    GoolObj *crash;

    entry = &g_obj_bounds.bounds[g_obj_bounds.count++];
    entry->obj = Ram_Addr(obj);
    if (obj->status_a & 0x8000) {
        Gool_ObjectLocalBound(obj, obj->vectors.scale);
        obj->status_a &= ~0x8000;
    }
    if (*RAM_AT(u8, obj->anim_seq) != 1) {
        entry->bound.x1 = obj->vectors.trans[0] + obj->bound[0];
        entry->bound.y1 = obj->vectors.trans[1] + obj->bound[1];
        entry->bound.z1 = obj->vectors.trans[2] + obj->bound[2];
        entry->bound.x2 = obj->vectors.trans[0] + obj->bound[3];
        entry->bound.y2 = obj->vectors.trans[1] + obj->bound[4];
        entry->bound.z2 = obj->vectors.trans[2] + obj->bound[5];
    } else {
        res = RAM_AT(u32, Ns_Lookup(RAM_AT(u32, obj->anim_seq) + 1));
        model = RAM_AT(s32, res[((u32)obj->anim_frame >> 8) + 4]);
        angle = obj->vectors.rot[1] & 0xFFF;
        if (angle == 0 && obj->vectors.scale[0] == 0x1000) {
            trans[0] = obj->vectors.trans[0] + model[11];
            trans[1] = obj->vectors.trans[1] + model[12];
            trans[2] = obj->vectors.trans[2] + model[13];
        } else {
            Gool_Transform((struct Vec3 *)(model + 11), (struct Vec3 *)obj->vectors.trans, (struct Vec3 *)obj->vectors.rot, (struct Vec3 *)obj->vectors.scale, (struct Vec3 *)trans);
        }
        if (angle < 0x200 || angle > 0xE00) {
            entry->bound.x1 = trans[0] + obj->bound[0];
            entry->bound.y1 = trans[1] + obj->bound[1];
            entry->bound.z1 = trans[2] + obj->bound[2];
            entry->bound.x2 = trans[0] + obj->bound[3];
            entry->bound.y2 = trans[1] + obj->bound[4];
            entry->bound.z2 = trans[2] + obj->bound[5];
        } else if (angle < 0x600) {
            entry->bound.x1 = trans[0] + obj->bound[2];
            entry->bound.x2 = trans[0] + obj->bound[5];
            entry->bound.y1 = trans[1] + obj->bound[1];
            entry->bound.y2 = trans[1] + obj->bound[4];
            entry->bound.z1 = trans[2] - obj->bound[3];
            entry->bound.z2 = trans[2] - obj->bound[0];
        } else if (angle < 0xA00) {
            entry->bound.x1 = trans[0] + obj->bound[0];
            entry->bound.x2 = trans[0] + obj->bound[3];
            entry->bound.y1 = trans[1] + obj->bound[1];
            entry->bound.y2 = trans[1] + obj->bound[4];
            entry->bound.z1 = trans[2] - obj->bound[5];
            entry->bound.z2 = trans[2] - obj->bound[2];
        } else {
            entry->bound.x1 = trans[0] - obj->bound[5];
            entry->bound.x2 = trans[0] - obj->bound[2];
            entry->bound.y1 = trans[1] + obj->bound[1];
            entry->bound.y2 = trans[1] + obj->bound[4];
            entry->bound.z1 = trans[2] + obj->bound[0];
            entry->bound.z2 = trans[2] + obj->bound[3];
        }
    }
    crash = RAM_AT(GoolObj, gp_crash_obj);
    if (crash->arg1 == obj->arg1) {
        player_bound.x1 = crash->vectors.trans[0] + crash->bound[0];
        player_bound.y1 = crash->vectors.trans[1] + crash->bound[1];
        player_bound.z1 = crash->vectors.trans[2] + crash->bound[2];
        player_bound.x2 = crash->vectors.trans[0] + crash->bound[3];
        player_bound.y2 = crash->vectors.trans[1] + crash->bound[4];
        player_bound.z2 = crash->vectors.trans[2] + crash->bound[5];
        if (Level_TestBoundIntersection((struct ZqBound *)&player_bound, (struct ZqBound *)&entry->bound) != 0) {
            Gool_Collide((struct CollideObj *)obj, (s32 *)&entry->bound, (struct CollideObj *)crash, (s32 *)&player_bound);
        } else {
            obj->collider = 0;
        }
    }
}

typedef struct GoolMoveParams {
    s32 accel;
    s32 max_speed;
    s32 word_8;
    s32 decel;
} GoolMoveParams;

typedef struct GoolDirEntry {
    s32 dir;
    s32 angle;
    s32 speed_scale;
} GoolDirEntry;

typedef struct GoolTimeState {
    u8 reserved_00[0x84];
    u32 word_84;
} GoolTimeState;

#define CRASH_EXT_GOOL_PHYSICS_3
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_PHYSICS_3

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define FRAME_TICKS (RAM_AT(GoolTimeState, g_db_cur[0])->word_84 < 0x67U ? RAM_AT(GoolTimeState, g_db_cur[0])->word_84 : 0x66)
#define SCALE_DT(x) (((s32)(FRAME_TICKS * (x))) / 1024)
#define CLAMP(x, min, max) ((x) >= (max) ? (max) : ((x) < (min) ? (min) : (x)))

void Gool_ObjectPhysics(GoolObj *obj) {
    GoolVec3 vel;
    GoolVec3 saved_trans;
    GoolVec3 path_trans;
    GoolMoveParams *params;
    u32 state_flags;
    u32 dir_idx;
    s32 speed_scale;
    s32 target_rot;
    s32 diff;
    s32 cos_val;

    if ((obj->status_b & 0x80) && g_game_substate == 0x100) {
        params = &g_accel_states[0];
        if (obj->state_flags & 4) {
            params = &g_accel_states[1];
            if (obj->status_a & 0x2000) {
                params += 4;
            }
        }
        if (obj->invincibility_state == 5) {
            params = &g_accel_states[4];
        }
        if (obj->state_flags & 8) {
            params = &g_accel_states[2];
            if (obj->status_a & 0x2000) {
                params += 4;
            }
        }
        state_flags = obj->state_flags;
        if (state_flags & 0x10) {
            params = &g_accel_states[3];
        }
        dir_idx = (g_gool_pad_buttons[0] & 0xF000) >> 12;
        speed_scale = g_move_states[dir_idx].speed_scale;
        if (g_move_states[dir_idx].dir != 8) {
            target_rot = (g_move_states[dir_idx].angle + g_cam_dir_rot_y[0]) & 0xFFF;
            diff = ABS(target_rot - obj->target_rot_y);
            if (diff > 0x800) {
                diff = 0x1000 - diff;
            }
            if (state_flags & 8) {
                cos_val = Math_Cos(diff);
                obj->speed += (SCALE_DT(params->accel) * (cos_val >> 6)) >> 6;
                if (obj->speed <= 0x100) {
                    obj->target_rot_y = target_rot;
                } else if (diff < 0x7C8) {
                    obj->target_rot_y = Gool_ObjectRotate(obj->target_rot_y, target_rot, 0xF00, NULL);
                } else {
                    obj->speed -= SCALE_DT(params->decel);
                    if (obj->speed < 0) {
                        obj->speed = 0;
                    }
                }
            } else if (obj->speed <= 0x100 || diff <= 0x400) {
                obj->target_rot_y = target_rot;
                obj->speed += SCALE_DT(params->accel);
            } else {
                obj->speed = 0;
            }
            if (obj->speed > params->max_speed) {
                obj->speed = params->max_speed;
            }
        } else {
            obj->speed -= SCALE_DT(params->decel);
            if (obj->speed < 0) {
                obj->speed = 0;
            }
        }
        speed_scale = (speed_scale * obj->speed) >> 8;
        obj->vel_x = ((Math_Sin(obj->target_rot_y) >> 4) * speed_scale) >> 8;
        obj->vel_z = ((Math_Cos(obj->target_rot_y) >> 4) * speed_scale) >> 8;
    }
    if ((obj->status_a & 1) && obj->event != 0x1200) {
        obj->status_a &= ~0x2000;
    }
    obj->status_a &= 0xFFCAA07E;
    if (obj->status_b & 1) {
        obj->vectors.rot[1] = Gool_ObjectRotate(obj->vectors.rot[1], obj->target_rot_y, obj->target_rot_x, (struct GoolObjView *)obj);
    }
    if (obj->status_b & 0x80000) {
        obj->vectors.rot[1] = Gool_ObjectRotate2(obj->vectors.rot[1], obj->target_rot_y, obj->target_rot_x, (struct GoolObjView *)obj);
    }
    if (obj->status_b & 0x2000) {
        obj->vectors.rot[0] = Gool_ObjectRotate(obj->vectors.rot[0], obj->target_rot_z, obj->ang_vel_x, NULL);
    }
    if (RAM_AT(GoolCodeHeader, RAM_AT(GoolExecutable, obj->exec)->header)->word_00 != 5) {
        obj->collider = 0;
    }
    if (obj->status_b & 0x40) {
        vel.x = SCALE_DT(obj->vel_x);
        vel.y = SCALE_DT(obj->vel_y);
        vel.z = SCALE_DT(obj->vel_z);
        saved_trans = *(GoolVec3 *)obj->vectors.trans;
        if (obj->status_b & 8) {
            obj->event = 0xFF;
            Solid_TransSmoothStopAtSolid((struct SubObj *)obj, (struct ZqVec *)&vel, (struct SubWorkspace *)g_solid_wall_plot_buf);
        } else {
            obj->vectors.trans[0] += vel.x;
            obj->vectors.trans[1] += vel.y;
            obj->vectors.trans[2] += vel.z;
        }
        if (obj->status_b & 0x8000) {
            Gool_ObjectOrientOnPath(obj, 0, (s32 *)&path_trans);
            obj->floor_y = path_trans.y;
            if (obj->status_a & 0x200) {
                *(GoolVec3 *)obj->vectors.trans = saved_trans;
            }
        }
        if (obj->status_b & 0x1000) {
            obj->vel_x = CLAMP(obj->vel_x, -obj->target_rot_z, obj->target_rot_z);
            obj->vel_y = CLAMP(obj->vel_y, -obj->target_rot_z, obj->target_rot_z);
            obj->vel_z = CLAMP(obj->vel_z, -obj->target_rot_z, obj->target_rot_z);
        }
        if (obj->status_b & 0x4000) {
            if (obj->vectors.trans[1] <= obj->floor_y) {
                obj->vectors.trans[1] = obj->floor_y;
                obj->status_a |= 1;
                obj->floor_time = g_level_zone_flags[0];
                if (obj->vel_y < 0) {
                    obj->floor_vel_y = obj->vel_y;
                    obj->vel_y = 0;
                }
            }
        }
    }
    if (obj->status_b & 0x20) {
        obj->vel_y -= FRAME_TICKS * 4000;
        if (obj->vel_y < -0x2EE000) {
            obj->vel_y = -0x2EE000;
        }
    }
    if ((obj->status_b & 0x10) && RAM_AT(GoolObj, gp_crash_obj)->arg1 != obj->arg1 &&
        Math_OutOfRange((struct ObjectFlagsView *)obj, (struct Vec3 *)obj->vectors.trans, (struct Vec3 *)RAM_AT(GoolObj, gp_crash_obj)->vectors.trans, 0x7D000, 0xAF000, 0x7D000) == 0) {
        Gool_ObjectControlDir(obj);
    }
}

typedef struct SVec3 {
    s16 x, y, z;
} SVec3;

