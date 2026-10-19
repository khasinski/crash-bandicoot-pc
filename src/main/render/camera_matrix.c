#include "common.h"
#include "gpu.h"

#define CRASH_EXT_RENDER_CAMERA_MATRIX_0
#include "render_externs.h"
#undef CRASH_EXT_RENDER_CAMERA_MATRIX_0

typedef struct Copy32 {
    s32 w[8];
} Copy32;

typedef struct Copy12 {
    s32 w[3];
} Copy12;

typedef struct ShortMat {
    s16 m[9];
} ShortMat;

#define CRASH_EXT_RENDER_CAMERA_MATRIX_1
#include "render_externs.h"
#undef CRASH_EXT_RENDER_CAMERA_MATRIX_1

typedef struct ZoneCamData {
    u8 reserved_000[0x2FC];
    u32 flags;
    u8 reserved_300[0x314 - 0x300];
    u8 bg_color[3];
} ZoneCamData;

typedef struct ZoneCamEntry {
    u8 reserved_00[0x10];
    RAM_REF(ZoneCamData) data;
} ZoneCamEntry;

typedef struct CamBlock {
    Copy32 m804;
    u8 reserved_824[0x80057888 - 0x80057824];
    Copy12 m888;
    u8 reserved_894[0x80057914 - 0x80057894];
    RAM_REF(ZoneCamEntry) z914;
    u8 reserved_918[0x80057938 - 0x80057918];
    s32 w938;
} CamBlock;

#define CRASH_EXT_RENDER_CAMERA_MATRIX_2
#include "render_externs.h"
#undef CRASH_EXT_RENDER_CAMERA_MATRIX_2

typedef struct WgeoHeader {
    s32 x;
    s32 y;
    s32 z;
    s32 word_0c;
    s32 word_10;
    s32 word_14;
    u32 tpage_count;
    s32 flags;
    u32 tpages[8];
} WgeoHeader;

typedef struct ZoneWorld {
    u32 eid;
    s32 trans[3];
    RAM_REF(WgeoHeader) wgeo;
    RAM_REF(void) item1;
    RAM_REF(void) item2;
    RAM_REF(void) polys;
    RAM_REF(void) tpages[8];
} ZoneWorld;

typedef struct ZoneHeader9A2C {
    s32 world_count;
    ZoneWorld worlds[8];
} ZoneHeader9A2C;

void Gfx_UpdateMatrices(void)
{
    ZoneCamData *zone;
    s16 sp10[16];
    s32 sp30[4];
    s16 s0_1, s0_2, s0_3, s0_4, v0, v0_2, v0_3, v0_4;
    s32 val;
    Copy32 *cam;
    s16 *m;
    s32 *t1;
    s32 *s0_5;

    Level_Noop();
    s0_1 = Math_Sin(-g_cam_rot_z);
    v0 = Math_Cos(-g_cam_rot_z);
    cam = (Copy32 *)&g_cam_trans_x;
    m = (s16 *)(cam - 5);
    *m = v0;
    g_m_cam_rot_01 = -s0_1;
    g_m_cam_rot_02 = 0;
    g_m_cam_rot_10 = s0_1;
    g_m_cam_rot_11 = v0;
    g_m_cam_rot_12 = 0;
    g_m_cam_rot_20 = 0;
    g_m_cam_rot_21 = 0;
    g_m_cam_rot_22 = 0x1000;

    s0_2 = Math_Sin(-g_cam_rot_x);
    v0_2 = Math_Cos(-g_cam_rot_x);
    sp10[0] = 0x1000;
    sp10[1] = 0;
    sp10[2] = 0;
    sp10[3] = 0;
    sp10[4] = v0_2;
    sp10[5] = -s0_2;
    sp10[6] = 0;
    sp10[7] = s0_2;
    sp10[8] = v0_2;
    MulMatrix((MATRIX *)m, (MATRIX *)sp10);

    s0_3 = Math_Sin(-g_cam_rot_y);
    v0_3 = Math_Cos(-g_cam_rot_y);
    sp10[0] = v0_3;
    sp10[1] = 0;
    sp10[2] = s0_3;
    sp10[3] = 0;
    sp10[4] = 0x1000;
    sp10[5] = 0;
    sp10[6] = -s0_3;
    sp10[7] = 0;
    sp10[8] = v0_3;
    MulMatrix((MATRIX *)m, (MATRIX *)sp10);

    zone = RAM_AT(ZoneCamData, RAM_AT(ZoneCamEntry, g_m_light.z914)->data);
    SetBackColor(zone->bg_color[0], zone->bg_color[1], zone->bg_color[2]);
    sp30[0] = *(s32 *)cam >> 8;
    sp30[1] = g_cam_trans_y >> 8;
    sp30[2] = g_cam_trans_z >> 8;
    TransMatrix((MATRIX *)m, (VECTOR *)sp30);

    (*(s16 *)g_m_view) = g_m_rot;
    g_m_view_01 = g_m_cam_rot_01;
    g_m_view_02 = g_m_cam_rot_02;
    g_m_view_10 = -(g_m_cam_rot_10 * 5) >> 3;
    g_m_view_11 = -(g_m_cam_rot_11 * 5) >> 3;
    g_m_view_12 = -(g_m_cam_rot_12 * 5) >> 3;
    g_m_view_20 = -g_m_cam_rot_20;
    g_m_view_21 = -g_m_cam_rot_21;
    g_m_view_22 = -g_m_cam_rot_22;

    g_m_light.m804 = *(cam - 4);

    if (RAM_AT(ZoneCamData, RAM_AT(ZoneCamEntry, g_m_light.z914)->data)->flags & 0x1000) {
        *(((s32 *)cam) + 10) = __builtin_abs(((g_draw_stamp[0] % 128) - 64) * 800) + 0xDC1E0;
        s0_4 = Math_Sin(0x7D);
        v0_4 = Math_Cos(0x7D);
        *(((s16 *)cam) - 0x20) = 0x1000;
        *(((s16 *)cam) - 0x1F) = 0;
        *(((s16 *)cam) - 0x1E) = 0;
        *(((s16 *)cam) - 0x1D) = 0;
        *(((s16 *)cam) - 0x1C) = v0_4;
        *(((s16 *)cam) - 0x1B) = -s0_4;
        *(((s16 *)cam) - 0x1A) = 0;
        ((ShortMat *)(cam - 2))->m[7] = s0_4;
        ((ShortMat *)(cam - 2))->m[8] = v0_4;

        g_m_wgeo_view = g_m_color;
        g_m_wgeo_view_01 = g_m_color_01;
        g_m_wgeo_view_02 = g_m_color_02;
        g_m_wgeo_view_10 = -(g_m_color_10 * 5) >> 3;
        g_m_wgeo_view_11 = -(g_m_color_11 * 5) >> 3;
        g_m_wgeo_view_12 = -(g_m_color_12 * 5) >> 3;
        g_m_wgeo_view_20 = -g_m_color_20;
        g_m_wgeo_view_21 = -g_m_color_21;
        g_m_wgeo_view_22 = -g_m_color_22;
    } else {
        *(cam - 2) = *(cam - 5);
        *(cam - 1) = *(cam - 4);
    }

    SetGeomOffset(g_gfx_debug_flag[0], (g_cam_mat_dirty_flag >> 8) + g_item_pool1[0]);
    val = g_cam_mat_dirty_flag >> 8;
    if (val == 0) {
        g_cam_mat_dirty_flag = 0;
    } else {
        if (val > 0) {
            val = -val + 1;
        } else {
            val = ~val;
        }
        g_cam_mat_dirty_flag = val << 8;
    }

    t1 = &g_cam_trans_x.w[0];
    g_title_pause_state = *t1;
    g_cam_prev_trans_y = g_cam_trans_y;
    g_cam_prev_trans_z = g_cam_trans_z;
    g_saved_item_pool1 = g_cam_rot_x;
    g_saved_item_pool2 = g_cam_rot_y;
    g_cam_prev_rot_z = g_cam_rot_z;

    if (!(RAM_AT(ZoneCamData, RAM_AT(ZoneCamEntry, g_m_light.z914)->data)->flags & 0x1000)) {
        g_m_light.m888 = *(Copy12 *)t1;
        g_cam_dir_rot_y.w[0] = g_m_light.w938 & 0xFFF;
    } else {
        g_cam_dir_rot_y.w[0] = 0;
    }

    s0_5 = &g_cam_dir_rot_y.w[0];
    g_ticks_cur_frame = *s0_5;
    g_cam_sin_rot_y = Math_Sin(*s0_5) >> 4;
    g_cam_dir_vec_y = 0;
    g_cam_cos_rot_y = Math_Cos(*s0_5) >> 4;
    Gfx_LoadWorldsFog(RAM_AT(ZoneHeader9A2C, RAM_AT(ZoneCamEntry, g_m_light.z914)->data));
}
