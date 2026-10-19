#include "common.h"
#include "zone.h"

typedef struct {
    s16 v[3][3];
    s16 b[3];
    s16 c[3][3];
    s16 a[3];
} LightMat;

typedef struct LevelHeader {
    s32 word_0;
    s32 level_id;
} LevelHeader;

#define CRASH_EXT_ENGINE_LEVEL_EFFECTS_0
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_LEVEL_EFFECTS_0

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
    ResourceEntry *entry;
    u8 reserved_2c[0x4C];
    struct SaveObj *collider;
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

#define CRASH_EXT_ENGINE_LEVEL_EFFECTS_1
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_LEVEL_EFFECTS_1

s32 gp_shader_prev_mode = -1;
s32 gp_octree_root = 0;

static s32 gp_shader_param_0;
static s32 gp_shader_param_1;
static s32 gp_shader_param_2;
static s32 gp_shader_param_3;
static s32 gp_shader_param_4;
static s32 gp_shader_param_5;
static s32 gp_shader_param_6;
static s32 gp_shader_param_7;
static s32 gp_shader_param_8;
static s32 gp_shader_param_9;
static s32 gp_shader_param_10;
static s32 gp_shader_param_11;
static s32 gp_shader_param_12;
static s32 gp_shader_param_13;
static s32 gp_shader_param_14;
static s32 gp_shader_param_15;
static s32 gp_shader_param_16;
static s32 gp_shader_param_17;
static s32 gp_shader_param_vec[2];

void Shader_ParamsUpdateRipple(s32 arg0)
{
    switch (g_ns_page_pool[0]->level_id) {
    case 0xF:
    case 0x18:
        gp_shader_ripple_speed = 0xA;
        gp_shader_ripple_amp = 0x7F;
        break;
    case 0x17:
        gp_shader_ripple_speed = 0xA;
        gp_shader_ripple_amp = 0x7F;
        break;
    case 0x24:
    case 0x33:
        gp_shader_ripple_speed = 4;
        gp_shader_ripple_amp = 0x7F;
        break;
    default:
        gp_shader_ripple_speed = 1;
        gp_shader_ripple_amp = 0x17;
        break;
    }
    if (arg0 != 0) {
        Gfx_TransformWorldsRipple(0);
    }
}

void Shader_ParamsUpdate(s32 arg0)
{
    u8 name[8];
    s32 sp18;
    s32 sp1C;
    s32 v1, v2, v3;

    if (arg0 != 0) {
        gp_ripple_amp = 0x800;
        gp_ripple_freq = 0xFF;
        gp_shader_tint_r = 0xFF;
        gp_shader_tint_g = 0xFF;
        gp_lightning_state = 0x800;
        gp_fade_step = 0;
        gp_shader_color_g = 0;
        gp_shader_color_b = 0;
        gp_octree_root = 0;
        gp_shader_prev_mode = -1;
        switch (g_ns_page_pool[0]->level_id) {
        case 0x21:
            gp_ripple_amp = 0;
            gp_ripple_freq = 0xC8;
            gp_shader_tint_r = 0x50;
            gp_shader_tint_g = 0;
            break;
        case 0x20:
        case 0x23:
            gp_ripple_amp = 0;
            gp_lightning_state = 0x640;
            gp_shader_param_5 = 0xA5;
            gp_shader_param_6 = 0x5A;
            gp_shader_param_7 = 0x64;
            gp_shader_param_8 = 0xFF;
            gp_shader_param_9 = 0x4B;
            gp_shader_param_10 = 0;
            gp_shader_param_2 = 0xA5;
            gp_shader_param_3 = 0x5A;
            gp_shader_param_4 = 0x64;
            break;
        case 5:
            gp_ripple_amp = 0;
            gp_ripple_freq = 0xEE;
            gp_shader_tint_r = 0xEE;
            gp_shader_tint_g = 0x3C;
            break;
        case 0x13:
            gp_ripple_amp = 0;
            gp_ripple_freq = 0;
            gp_shader_tint_r = 0xF0;
            gp_shader_tint_g = 0xFF;
            break;
        case 0xA:
        case 0x1C:
        case 0x1D:
            gp_ripple_amp = 0;
            gp_ripple_freq = 0xFF;
            gp_shader_tint_r = 0x64;
            gp_shader_tint_g = 0;
            break;
        case 3:
        case 6:
        case 7:
        case 0x37:
            gp_ripple_amp = 0;
            gp_ripple_freq = 0xFF;
            gp_shader_tint_r = 0x2B;
            gp_shader_tint_g = 0xB;
            break;
        case 0x28:
        case 0x2A:
            gp_shader_param_11 = 0;
            gp_dark_cone_rot = 0;
            gp_shader_dark_radius = 0xA;
            gp_shader_param_14 = 0xFFF;
            gp_shader_param_17 = 0x7D0;
            gp_shader_dark_z_bias = -0x36B0;
            break;
        }
    } else {
        switch (g_ns_page_pool[0]->level_id) {
        case 0x28:
        case 0x2A:
            if ((v1 = (s32)g_shader_obj_color_scale) != 0) {
                gp_dark_target_x = ((SaveObj *)v1)->transform.trans.x;
                gp_dark_target_y = ((SaveObj *)v1)->transform.trans.y;
                gp_dark_target_z = ((SaveObj *)v1)->transform.trans.z;
            } else {
                gp_dark_target_x = gp_crash_obj->transform.trans.x;
                gp_dark_target_y = gp_crash_obj->transform.trans.y;
                gp_dark_target_z = gp_crash_obj->transform.trans.z;
            }
            v1 = g_pause_obj;
            if (v1 != 0) {
                if (v1 != gp_shader_param_11) {
                    gp_shader_param_12 = -0x1F40;
                    gp_shader_param_13 = -0x1F4;
                    gp_shader_param_15 = 0x4B;
                    gp_shader_param_16 = -0x4B;
                }
            } else {
                if (gp_shader_param_11 != 0) {
                    gp_shader_param_12 = 0xFFF;
                    gp_shader_param_13 = 0x64;
                    gp_shader_param_15 = 0x7D0;
                    gp_shader_param_16 = 0x14;
                }
            }
            gp_shader_param_11 = v1;
            if (gp_shader_param_14 != gp_shader_param_12) {
                if ((gp_shader_param_14 < gp_shader_param_12 && gp_shader_param_14 + gp_shader_param_13 >= gp_shader_param_12) ||
                    (gp_shader_param_14 > gp_shader_param_12 && gp_shader_param_14 + gp_shader_param_13 <= gp_shader_param_12)) {
                    gp_shader_param_13 = 0;
                    gp_shader_param_14 = gp_shader_param_12;
                } else {
                    gp_shader_param_14 += gp_shader_param_13;
                }
            }
            if (gp_shader_param_17 != gp_shader_param_15) {
                if ((gp_shader_param_17 < gp_shader_param_15 && gp_shader_param_17 + gp_shader_param_16 >= gp_shader_param_15) ||
                    (gp_shader_param_17 > gp_shader_param_15 && gp_shader_param_17 + gp_shader_param_16 <= gp_shader_param_15)) {
                    gp_shader_param_16 = 0;
                    gp_shader_param_17 = gp_shader_param_15;
                } else {
                    gp_shader_param_17 += gp_shader_param_16;
                }
            }
            gp_shader_dark_intensity = gp_shader_param_14;
            gp_shader_depth_cue_flag = gp_shader_param_17;
            break;
        case 0x21:
            switch (gp_octree_root) {
            case 0:
                gp_shader_param_0 = Math_Randb(1000);
                gp_shader_param_1 = (gp_shader_param_0 - gp_lightning_state) / 4;
                gp_lightning_state += gp_shader_param_1;
                break;
            case 1:
            case 2:
            case 3:
                gp_lightning_state += gp_shader_param_1;
                break;
            }
            gp_octree_root = ++gp_octree_root % 4;
            break;
        case 5:
        case 0xA:
        case 0x1C:
        case 0x1D:
            switch (gp_octree_root) {
            case 0:
                gp_shader_param_0 = Math_Randb(0x5DC);
                gp_lightning_state += (gp_shader_param_0 - gp_lightning_state) / 2;
                break;
            case 1:
                gp_lightning_state = gp_shader_param_0;
                break;
            }
            gp_octree_root = (gp_octree_root + 1) % 2;
            break;
        case 0x20:
        case 0x23:
            switch (gp_octree_root) {
            case 0:
                gp_shader_param_0 = Math_Randb(100);
                gp_shader_param_2 = gp_shader_param_5 + (gp_shader_param_8 - gp_shader_param_5) * gp_shader_param_0 / 100;
                gp_shader_param_3 = gp_shader_param_6 + (gp_shader_param_9 - gp_shader_param_6) * gp_shader_param_0 / 100;
                gp_shader_param_4 = gp_shader_param_7 + (gp_shader_param_10 - gp_shader_param_7) * gp_shader_param_0 / 100;
                gp_ripple_freq += (gp_shader_param_2 - gp_ripple_freq) / 2;
                gp_shader_tint_r += (gp_shader_param_3 - gp_shader_tint_r) / 2;
                gp_shader_tint_g += (gp_shader_param_4 - gp_shader_tint_g) / 2;
                break;
            case 1:
                gp_ripple_freq = gp_shader_param_2;
                gp_shader_tint_r = gp_shader_param_3;
                gp_shader_tint_g = gp_shader_param_4;
                break;
            }
            gp_octree_root = (gp_octree_root + 1) % 2;
            break;
        case 3:
        case 6:
        case 7:
        case 0x37:
            if (g_shader_lightning_ramp[gp_octree_root] == -1) {
                gp_octree_root = 0;
            }
            gp_lightning_state = g_shader_lightning_ramp[gp_octree_root++];
            break;
        case 0x13:
            if (g_shader_lightning_ramp[gp_octree_root] == -1) {
                gp_octree_root = 0;
            }
            gp_lightning_state = g_shader_lightning_ramp[gp_octree_root++] >> 1;
            break;
        case 0x1B:
        case 0x22:
        case 0x2E:
            if (gp_shader_prev_mode != -1) {
                v2 = g_shader_color_table_ptrs[gp_shader_prev_mode][gp_octree_root];
                if (v2 == -1) {
                    *(volatile s32 *)&gp_ripple_amp = 0;
                    gp_lightning_state = 0;
                    gp_shader_prev_mode = -1;
                } else {
                    gp_ripple_amp = v2;
                    gp_lightning_state = g_shader_color_table_ptrs[gp_shader_prev_mode][gp_octree_root++];
                }
            } else {
                gp_ripple_amp = 0;
                gp_lightning_state = 0;
                if (Math_Randb(1000) < 0x19) {
                    if (g_ns_page_pool[0]->level_id == 0x22 || g_ns_page_pool[0]->level_id == 0x2E) {
                        switch (Math_Randb(3)) {
                        case 0:
                            gp_shader_prev_mode = 5;
                            break;
                        case 1:
                            gp_shader_prev_mode = 4;
                            break;
                        case 2:
                            gp_shader_prev_mode = Math_Randb(4);
                            break;
                        }
                    } else {
                        gp_shader_prev_mode = Math_Randb(6);
                    }
                    gp_octree_root = 0;
                    if (gp_next_lid == 0 &&
                        (g_ns_page_pool[0]->level_id == 0x1B || g_ns_page_pool[0]->level_id == 0x22 || g_ns_page_pool[0]->level_id == 0x2E) &&
                        (v3 = frame_counter) - gp_shader_param_vec[1] > 0x1800) {
                        __builtin_memcpy(name, gp_str_light_name_tmpl, 6);
                        for (sp18 = 0; sp18 < 1; sp18++) {
                            gp_shader_param_vec[sp18 + 1] = gp_shader_param_vec[sp18];
                        }
                        gp_shader_param_vec[0] = v3;
                        name[2] += Math_Randb(3);
                        sp1C = Ns_StringToEID(name);
                        sp18 = (Math_Randb(0x4CC) + 0xD99) >> 3;
                        Audio_Control(0, 1, &sp18, 0);
                        sp18 = Math_Randb(0xF) + 1;
                        Audio_Control(0, 7, &sp18, 0);
                        sp18 = Math_Randb(0x14) + 0x14;
                        if (Math_Randb(100) >= 0x15) {
                            sp18 += Math_Randb(0x32);
                        }
                        Audio_VoiceCreate(0, &sp1C, sp18 * 0x3FFF / 100);
                    }
                }
            }
            break;
        }
    }
}
