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

#define CRASH_EXT_LEVEL_LIGHT
#include "level_externs.h"
#undef CRASH_EXT_LEVEL_LIGHT

void Zone_ColorsScale(LightMat *arg0, LightMat *arg1, s32 arg2, s32 arg3, s32 arg4)
{
    s32 i;

    arg1->b[0] = (arg0->b[0] * ((arg2 << 12) / 100)) / 4096;
    arg1->b[1] = (arg0->b[1] * ((arg3 << 12) / 100)) / 4096;
    arg1->b[2] = (arg0->b[2] * ((arg4 << 12) / 100)) / 4096;
    arg1->a[0] = arg0->a[0];
    arg1->a[1] = arg0->a[1];
    arg1->a[2] = arg0->a[2];
    for (i = 0; i < 3; i++) {
        arg1->v[i][0] = (arg0->v[i][0] * ((arg2 << 12) / 100)) / 4096;
        arg1->v[i][1] = (arg0->v[i][1] * ((arg3 << 12) / 100)) / 4096;
        arg1->v[i][2] = (arg0->v[i][2] * ((arg4 << 12) / 100)) / 4096;
        arg1->c[i][0] = arg0->c[i][0];
        arg1->c[i][1] = arg0->c[i][1];
        arg1->c[i][2] = arg0->c[i][2];
    }
}

void Zone_ColorsScaleByNode(s32 idx, LightMat *src, LightMat *dst)
{
    if (idx >= 0x27) {
        Zone_ColorsScale(src, dst, 0, 0, 0);
        if (RAM_AT(LevelHeader, g_ns_page_pool[0])->level_id == 0x13) {
            if (idx == 0x28) {
                Zone_ColorsScale(src, dst, 0x64, 0x64, 0x64);
                dst->a[0] = 0;
                dst->a[1] = 0xFF;
                dst->a[2] = 0xFF;
                dst->c[0][0] = 0;
                dst->c[0][1] = 0x3B0;
                dst->c[0][2] = 0x3B0;
                dst->c[1][0] = 0;
                dst->c[1][1] = 0xF9;
                dst->c[1][2] = 0xFF;
                dst->c[2][0] = 0;
                dst->c[2][1] = 0x64;
                dst->c[2][2] = 0xFF;
            }
        } else if (RAM_AT(LevelHeader, g_ns_page_pool[0])->level_id == 7) {
            if (idx == 0x28) {
                dst->b[0] = 0;
                dst->b[1] = 0xFF;
                dst->b[2] = 0xFF;
                dst->a[0] = 0xFF;
                dst->a[1] = 0xFF;
                dst->a[2] = 0xFF;
                dst->v[0][0] = 0;
                dst->v[0][1] = -0x2199;
                dst->v[0][2] = 0;
                dst->v[1][0] = -0xEE1;
                dst->v[1][1] = -0x68F;
                dst->v[1][2] = 0xA3D;
                dst->v[2][0] = 0xDEB;
                dst->v[2][1] = 0x1333;
                dst->v[2][2] = -0x11E;
                dst->c[0][0] = 0xC0;
                dst->c[0][1] = 0xFF;
                dst->c[0][2] = 0xC0;
                dst->c[1][0] = 0xE0;
                dst->c[1][1] = 0x190;
                dst->c[1][2] = 0xE0;
                dst->c[2][0] = 0x104;
                dst->c[2][1] = 0xF0;
                dst->c[2][2] = 0xF0;
            }
        } else if (RAM_AT(LevelHeader, g_ns_page_pool[0])->level_id == 3) {
            if (idx == 0x28) {
                dst->b[0] = 0;
                dst->b[1] = 0xFF;
                dst->b[2] = 0xFF;
                dst->a[0] = 0xFF;
                dst->a[1] = 0xFF;
                dst->a[2] = 0xFF;
                dst->v[0][0] = 0;
                dst->v[0][1] = -0x2199;
                dst->v[0][2] = 0;
                dst->v[1][0] = -0xEE1;
                dst->v[1][1] = -0x68F;
                dst->v[1][2] = 0xA3D;
                dst->c[0][0] = 0;
                dst->c[0][1] = 0xFF;
                dst->c[0][2] = 0;
                dst->c[1][0] = 0x58;
                dst->c[1][1] = 0x27D;
                dst->c[1][2] = 0x5A;
                dst->v[2][0] = 0xDEB;
                dst->v[2][1] = 0x1333;
                dst->v[2][2] = -0x11E;
                dst->c[2][0] = 0x11C;
                dst->c[2][1] = 0x80;
                dst->c[2][2] = 0x80;
            }
        } else if (RAM_AT(LevelHeader, g_ns_page_pool[0])->level_id == 0x1C || RAM_AT(LevelHeader, g_ns_page_pool[0])->level_id == 0x1D) {
            switch (idx) {
            case 0x28:
                Zone_ColorsScale(src, dst, 0x32, 0x64, 0x64);
                break;
            case 0x29:
                Zone_ColorsScale(src, dst, 0x4B, 0x64, 0x64);
                break;
            case 0x2A:
                Zone_ColorsScale(src, dst, 0x64, 0x64, 0x64);
                break;
            case 0x2B:
                Zone_ColorsScale(src, dst, 0x7D, 0x64, 0x64);
                break;
            case 0x2C:
                Zone_ColorsScale(src, dst, 0x96, 0x64, 0x64);
                break;
            }
        }
        if (idx < 0x28) {
            Zone_ColorsScale(src, dst, 0x64, 0x64, 0x64);
        } else if (idx >= 0x30) {
            if (idx < 0x40) {
                Zone_ColorsScale(src, dst, g_zone_octree_child_offsets[idx], g_zone_octree_child_offsets[idx], g_zone_octree_child_offsets[idx]);
            } else {
                Zone_ColorsScale(src, dst, 0x64, 0x64, 0x64);
            }
        }
    } else {
        Zone_ColorsScale(src, dst, 0x64, 0x64, 0x64);
    }
}
