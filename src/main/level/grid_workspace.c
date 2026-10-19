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
    struct {
        s32 type;
        s32 category;
    } *header;
} SubExec;

typedef struct SubObj {
    u8 reserved_00[8];
    ZqBound bound;
    RAM_REF(SubExec) word_20;
    u8 reserved_24[4];
    RAM_REF(ResourceEntry) entry;
    u8 reserved_2c[0x64 - 0x2C];
    RAM_REF(struct SubObj) parent;
    u8 reserved_68[0x78 - 0x68];
    RAM_REF(struct SubObj) collider;
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
    RAM_REF(u16) word_110;
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

typedef struct SubBox {
    ZqBound bound;
    RAM_REF(SubObj) obj;
} SubBox;

typedef struct SubBoxList {
    SubBox boxes[96];
    s32 count;
} SubBoxList;

#define CRASH_EXT_LEVEL_GRID_WORKSPACE
#include "level_externs.h"
#undef CRASH_EXT_LEVEL_GRID_WORKSPACE

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

void Solid_PlotObjWalls(ZqVec *v, SubObj *obj, SubWorkspace *ws, s32 flag)
{
    ZqBound bound1;
    ZqBound bound2;
    ZqBound bound3;
    ZqVec diff;
    s32 i, vx, vz, no_grid;
    s32 x, x1, x2, z, z1, z2;
    SubObj *collider, *cand_obj;
    SubBox *box;

    no_grid = !(((ZoneHeader *)((ZoneEntry *)level_position.zone)->items[0])->flags & 0x100000);
    vx = v->x;
    vz = v->z;
    if (g_obj_bounds.count != 0) {
        collider = obj->collider;
        bound1.p1.x = v->x + obj->bound.p1.x;
        bound1.p1.y = v->y + obj->bound.p1.y;
        bound1.p1.z = v->z + obj->bound.p1.z;
        bound1.p2.x = v->x + obj->bound.p2.x;
        bound1.p2.y = v->y + obj->bound.p2.y;
        bound1.p2.z = v->z + obj->bound.p2.z;
        bound2.p1.x = bound1.p1.x - 0x6400;
        bound2.p1.y = bound1.p1.y;
        bound2.p1.z = bound1.p1.z - 0x6400;
        bound2.p2.x = bound1.p2.x + 0x6400;
        bound2.p2.y = bound1.p2.y;
        bound2.p2.z = bound1.p2.z + 0x6400;
        ws->word_104c += 0x100;
        box = g_obj_bounds.boxes;
        for (i = g_obj_bounds.count - 1; i >= 0; i--, box++) {
            cand_obj = box->obj;
            if ((cand_obj != collider || !flag || box->bound.p2.y > bound2.p1.y)
                && Level_TestBoundIntersection_inline(&bound2, &box->bound)) {
                bound3.p1.x = box->bound.p1.x + cand_obj->head_turn;
                bound3.p1.y = box->bound.p1.y;
                bound3.p1.z = box->bound.p1.z + cand_obj->head_turn;
                bound3.p2.x = box->bound.p2.x - cand_obj->head_turn;
                bound3.p2.y = box->bound.p2.y;
                bound3.p2.z = box->bound.p2.z - cand_obj->head_turn;
                if (no_grid
                    && (cand_obj->status_b & 0x10000)
                    && (!(((obj->flags120 & 0x10) || obj->anim_state == 5)
                          && cand_obj->word_20->header->category == 0x300
                          && (!(cand_obj->status_c & 0x1012) || (cand_obj->flags120 & 0x10020))))
                    && bound2.p1.y <= box->bound.p2.y
                    && bound2.p2.y > box->bound.p1.y
                    && (cand_obj->word_20->header->type != 0xB
                        || (diff.x = (v->x - cand_obj->trans.x) >> 8,
                            diff.z = (v->z - cand_obj->trans.z) >> 8,
                            (SquareRoot0(diff.x * diff.x + diff.z * diff.z) << 8) > 0x19000))) {
                    x = ((bound3.p1.x - vx) << 2) / 0x2000;
                    x1 = x;
                    x2 = ((bound3.p2.x - vx) << 2) / 0x2000;
                    z1 = ((bound3.p1.z - vz) << 2) / 0x2000;
                    z = z1;
                    z2 = ((bound3.p2.z - vz) << 2) / 0x2000;
                    Solid_PlotWallB(x, z1, x2, z2, 1);
                    if (flag) {
                        for (; x < x2; x += 8) {
                            Solid_PlotWall(x, z);
                        }
                        for (x = x2; z < z2; z += 8) {
                            Solid_PlotWall(x, z);
                        }
                        for (z = z2; x > x1; x -= 8) {
                            Solid_PlotWall(x, z);
                        }
                        for (x = x1; z > z1; z -= 8) {
                            Solid_PlotWall(x, z);
                        }
                    }
                }
                if (flag && Level_TestBoundIntersection_inline(&bound1, &box->bound)) {
                    Gool_Collide(obj, &bound1, box->obj, &box->bound);
                }
            }
        }
    }
}

void Solid_PlotWalls(ZqVec *v, SubObj *obj, SubWorkspace *ws)
{
    u8 unused[0x30];
    s32 i, state, status_c, a, c;
    s32 *p1;
    u32 *p2;
    ZqBound *bound;

    if (!(((ZoneHeader *)((ZoneEntry *)level_position.zone)->items[0])->flags & 0x100000)) {
        i = 0x1F;
        p1 = &gp_grid_rows[0x1F];
        do {
            *p1 = 0;
            i--;
            p1--;
        } while (i >= 0);
        i = 0x7F;
        p2 = &gp_solid_scratch_grid[0x7F];
        do {
            *p2 = 0;
            i--;
            p2--;
        } while (i >= 0);
        bound = &ws->bound1;
        state = obj->anim_state;
        status_c = obj->status_c;
        if (state == 3 || state == 4 || (a = status_c & 2, state == 2)) {
            c = 2;
        } else {
            c = a;
        }
        Solid_RPlotQueryWalls(ws, bound, c,
                      obj->trans.y + gp_wall_bitmap, obj->trans.y + 0x3200, v->y + 0x29900, v->x, v->z);
    }
    Solid_PlotObjWalls(v, obj, ws, 1);
}

