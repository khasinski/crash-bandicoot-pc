#include "common.h"
#include "zone.h"

typedef struct ZqVec {
    s32 x, y, z;
} ZqVec;

typedef struct ZqBound {
    ZqVec p1;
    ZqVec p2;
} ZqBound;

typedef struct ZqRect {
    s32 x, y, z;
    s32 w, h, d;
} ZqRect;

typedef struct ZqZoneRect {
    s32 x, y, z;
    s32 w, h, d;
    u32 word_18;
    u16 root;
    s16 max_depth_x;
    s16 max_depth_y;
    s16 max_depth_z;
} ZqZoneRect;

typedef struct LevelStateView {
    u8 reserved_00[4];
    s32 level_id;
} LevelStateView;

typedef struct SubExec {
    u8 reserved_00[0x10];
    s32 *header;
} SubExec;

typedef struct SubObj {
    u8 reserved_00[8];
    ZqBound bound;
    SubExec *word_20;
    u8 reserved_24[4];
    ResourceEntry *entry;
    u8 reserved_2c[0x64 - 0x2C];
    struct SubObj *parent;
    u8 reserved_68[0x78 - 0x68];
    struct SubObj *collider;
    u8 reserved_7c[4];
    ZqVec trans;
    u8 reserved_8c[0xA8 - 0x8C];
    s32 vel_y;
    u8 reserved_ac[0xC8 - 0xAC];
    u32 status_a;
    u32 status_b;
    u32 status_c;
    u8 reserved_d4[0xFC - 0xD4];
    u32 arg1;
    u8 reserved_100[0x110 - 0x100];
    u16 *word_110;
    u8 reserved_114[0x120 - 0x114];
    u32 flags120;
    u8 reserved_124[4];
    s32 anim_state;
    u8 reserved_12c[4];
    u32 ground_time;
    s32 impact_vel_y;
    s32 offset138;
    s32 event;
    u8 reserved_140[0x148 - 0x140];
    s32 head_turn;
    u8 reserved_14c[0x158 - 0x14C];
    s32 word_158;
} SubObj;

typedef struct SubWsNode {
    u16 flags;
    u16 x;
    u16 y;
    u16 z;
} SubWsNode;

typedef struct SubWorkspace {
    SubWsNode nodes[0x200];
    s32 initialized;
    s32 count;
    ZqBound bound1;
    ZqBound bound2;
    s32 word_1038;
    s32 word_103c;
    s32 word_1040;
    s32 word_1044;
    s32 word_1048;
    s32 word_104c;
} SubWorkspace;

#define CRASH_EXT_LEVEL_GRID_SEARCH
#include "level_externs.h"
#undef CRASH_EXT_LEVEL_GRID_SEARCH

typedef struct SubBox {
    ZqBound bound;
    SubObj *obj;
} SubBox;

typedef struct SubBoxList {
    SubBox boxes[96];
    s32 count;
} SubBoxList;

static __inline__ s32 zone_rect_overlaps(ZqBound *b, ZqRect *r)
{
    if ((r->x << 8) < b->p2.x && (r->y << 8) < b->p2.y && (r->z << 8) < b->p2.z
        && ((r->x + r->w) << 8) >= b->p1.x && ((r->y + r->h) << 8) >= b->p1.y
        && ((r->z + r->d) << 8) >= b->p1.z) {
        return 1;
    }
    return 0;
}

static __inline__ s32 Level_TestBoundIntersection_inline(ZqBound *a, ZqBound *b)
{
    if (a->p1.y <= b->p2.y && a->p2.y > b->p1.y && a->p2.x > b->p1.x
        && a->p2.z > b->p1.z && a->p1.x <= b->p2.x && a->p1.z <= b->p2.z) {
        return 1;
    }
    return 0;
}

static __inline__ s32 Level_TestBoundInBound(ZqBound *a, ZqBound *b)
{
    if (a->p1.x >= b->p1.x && a->p1.y >= b->p1.y && a->p1.z >= b->p1.z
        && a->p2.x < b->p2.x && a->p2.y < b->p2.y && a->p2.z < b->p2.z) {
        return 1;
    }
    return 0;
}

static __inline__ void Solid_PlotWallB(s32 x1, s32 y1, s32 x2, s32 y2, s32 set)
{
    s32 *p;
    u32 mask;

    x2 += 16;
    mask = -1;
    if (x2 < 32) {
        if (x2 <= 0) {
            return;
        }
        mask = -1 << (32 - x2);
    }
    x1 += 15;
    if (x1 >= 32) {
        return;
    }
    if (x1 >= 0) {
        mask &= ~((s32)0x80000000 >> x1);
    }
    if (mask == 0) {
        return;
    }
    y1 += 16;
    y2 += 16;
    if (y1 < 0) {
        y1 = 0;
    }
    if (y2 > 32) {
        y2 = 32;
    }
    if (y1 >= 32) {
        return;
    }
    p = &gp_grid_rows[y1];
    if (set) {
        for (; y1 < y2; y1++, p++) {
            *p |= mask;
        }
    } else {
        mask = ~mask;
        for (; y1 < y2; y1++, p++) {
            *p &= mask;
        }
    }
}

static __inline__ s32 Solid_TestWallCell(s32 x, s32 y)
{
    if ((u32)x < 32 && (u32)y < 32 && (gp_grid_rows[y] & (0x80000000U >> x)) == 0) {
        return 1;
    }
    return 0;
}

s32 Solid_ReplotWalls(s32 set, s32 mode, ZqVec *v, SubObj *obj)
{
    ZqBound bound;
    ZqVec size;
    SubWorkspace *ws;
    SubWsNode *node;
    u16 max_depth_x, max_depth_y, max_depth_z, depth;
    u32 val;
    s32 i, count, type, x1, x2, z1, z2;

    i = 0;
    count = 0;
    ws = &g_solid_wall_plot_buf;
    for (; i < ws->count; i++) {
        node = &ws->nodes[i];
        if (node->flags == 0) {
            i++;
            size.x = node->x << 8;
            size.y = node->y << 8;
            size.z = node->z << 8;
            node = &ws->nodes[i];
            max_depth_x = node->x;
            max_depth_y = node->y;
            max_depth_z = node->z;
        } else {
            val = (node->flags >> 2) | 1;
            type = val ? (((val & 0xE) >> 1) + 1) : 0;
            if (type != 5 && type != 0) {
                switch (mode) {
                case 0:
                    if (type != 2) {
                        continue;
                    }
                    break;
                case 1:
                    if (type == 2) {
                        continue;
                    }
                    break;
                }
                bound.p1.y = (((s16)node->y << 4) + ws->bound1.p1.y) - v->y;
                bound.p2.y = (size.y >> (max_depth_y < depth ? max_depth_y : depth)) + bound.p1.y;
                switch (mode) {
                case 0:
                    if (bound.p1.y > 0x6400 || bound.p2.y < -0x19000) {
                        continue;
                    }
                    break;
                case 1:
                    if (bound.p2.y < 0) {
                        continue;
                    }
                    break;
                }
                bound.p1.x = (((s16)node->x << 4) + ws->bound1.p1.x) - v->x;
                bound.p1.z = (((s16)node->z << 4) + ws->bound1.p1.z) - v->z;
                depth = node->flags & 7;
                bound.p2.x = (size.x >> (max_depth_x < depth ? max_depth_x : depth)) + bound.p1.x;
                bound.p2.z = (size.z >> (max_depth_z < depth ? max_depth_z : depth)) + bound.p1.z;
                x1 = (bound.p1.x << 2) / 0x2000;
                x2 = (bound.p2.x << 2) / 0x2000;
                z1 = (bound.p1.z << 2) / 0x2000;
                z2 = (bound.p2.z << 2) / 0x2000;
                Solid_PlotWallB(x1, z1, x2, z2, set);
                count++;
            }
        }
    }
    return count;
}

s32 Solid_StopAtWalls(ZqVec *v, s32 gx, s32 gz, s32 *out_x, s32 *out_z, SubObj *obj, s32 depth)
{
    s32 i, dx, dz, res;
    s8 *p;

start:
    p = g_solid_grid_search_order;
    if (Solid_TestWallCell(gx, gz) || (((ZoneHeader *)((ZoneEntry *)level_position.zone)->items[0])->flags & 0x100000)) {
        *out_x = gx;
        *out_z = gz;
        return depth + 2;
    retry:
        gx = 16;
        gz = 16;
        depth++;
        goto start;
    }
    for (i = 0; i < 0x98; i++) {
        dx = p[0];
        dz = p[1];
        p += 2;
        if (Solid_TestWallCell(gx + dx, gz + dz)) {
            *out_x = gx + dx;
            *out_z = gz + dz;
            return depth + 2;
        }
        if (Solid_TestWallCell(gx + dx, gz - dz)) {
            *out_x = gx + dx;
            *out_z = gz - dz;
            return depth + 2;
        }
        if (Solid_TestWallCell(gx - dx, gz + dz)) {
            *out_x = gx - dx;
            *out_z = gz + dz;
            return depth + 2;
        }
        if (Solid_TestWallCell(gx - dx, gz - dz)) {
            *out_x = gx - dx;
            *out_z = gz - dz;
            return depth + 2;
        }
        if (dx != dz) {
            if (Solid_TestWallCell(gx + dz, gz + dx)) {
                *out_x = gx + dz;
                *out_z = gz + dx;
                return depth + 2;
            }
            if (Solid_TestWallCell(gx + dz, gz - dx)) {
                *out_x = gx + dz;
                *out_z = gz - dx;
                return depth + 2;
            }
            if (Solid_TestWallCell(gx - dz, gz + dx)) {
                *out_x = gx - dz;
                *out_z = gz + dx;
                return depth + 2;
            }
            if (Solid_TestWallCell(gx - dz, gz - dx)) {
                *out_x = gx - dz;
                *out_z = gz - dx;
                return depth + 2;
            }
        }
    }
    if ((gx != 16 || gz != 16)
        && (obj->collider == NULL || *obj->collider->word_20->header != 0x22)) {
        for (i = 0; i < 32; i++) {
            if (gp_grid_rows[i] != -1) {
                goto retry;
            }
        }
    }
    if (depth < 0x100
        && (obj->collider == NULL || *obj->collider->word_20->header != 0x22)) {
        if (Solid_ReplotWalls(0, 0, v, obj)) {
            Solid_ReplotWalls(1, 1, v, obj);
            Solid_PlotObjWalls(v, obj, &g_solid_wall_plot_buf, 0);
        }
        res = Solid_StopAtWalls(v, gx, gz, out_x, out_z, obj, 0x100);
        return res;
    }
    obj->status_a |= 0x100000;
    *out_x = 16;
    *out_z = 16;
    return 0;
}
