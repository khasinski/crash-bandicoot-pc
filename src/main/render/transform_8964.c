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
    s32 arg0_v;
    s32 arg1_v;
    s32 arg2_v;
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

typedef struct ZoneFogHeader {
    u8 reserved_00[0x2E8];
    s32 depth_offset;
} ZoneFogHeader;

typedef struct ZoneEntry8964 {
    u8 reserved_00[0x10];
    RAM_REF(ZoneFogHeader) data;
} ZoneEntry8964;

#define CRASH_EXT_RENDER_TRANSFORM_8964
#include "render_externs.h"
#undef CRASH_EXT_RENDER_TRANSFORM_8964

void Gfx_LoadWorlds(u8 *arg0, u32 *arg1, GoolObj *arg2) {
    ResourceEntry *res;
    void *data;

    res = Ns_Lookup(arg0 + 4);
    data = res->data;
    if (Gfx_CalcObjectMatrices(arg0, data, arg2, 1, NULL) != 0) {
        Gte_RTransformSvtx(
            arg0,
            arg1,
            res->word_14,
            data,
            arg2->vectors.scale[0],
            arg2->cam_zoom + (0x800 - g_view_proj[0] / 2),
            Gpu_GetCurPrimsTail(),
            g_wgeo_tex_anim_masks,
            0);
    }
}

void Gfx_TransformCvtx(u8 *arg0, u32 *arg1, GoolObj *arg2) {
    ResourceEntry *res;
    s32 sp28;
    void *data;
    s32 visible;

    res = Ns_Lookup(arg0 + 4);
    sp28 = 0;
    data = res->data;
    if (arg2->status_b & 0x200) {
        visible = Gte_RCalcSpriteRotMatrix(
            &arg2->vectors,
            &(*(CamTransformState *)g_m_view).vec888,
            1,
            0,
            &(*(CamTransformState *)g_m_view).mat844,
            g_view_proj[0],
            g_pads[0]->data->depth_offset >> 8,
            ((arg2->status_b >> 18) ^ 1) & 1);
    } else {
        visible = Gfx_CalcObjectMatrices(arg0, data, arg2, 0, &sp28);
    }
    if (visible != 0) {
        Gte_RTransformCvtx(
            arg0,
            arg1,
            res->word_14,
            data,
            arg2->vectors.scale[0],
            arg2->cam_zoom + (0x800 - g_view_proj[0] / 2),
            Gpu_GetCurPrimsTail(),
            g_wgeo_tex_anim_masks,
            sp28);
    }
}
