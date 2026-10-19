#include "common.h"

#define CRASH_EXT_GOOL_CHANGE_STATE_0
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_CHANGE_STATE_0

typedef struct GoolAnim {
    u8 type;
    u8 reserved[3];
    u32 eid;
} GoolAnim;

typedef struct SvtxFrame {
    u8 reserved_00[0x14];
    s32 box[6];
    s32 col_x;
    s32 col_y;
    s32 col_z;
} SvtxFrame;

typedef struct GoolHeader {
    u8 reserved_00[0x0C];
    s32 sp_offset;
} GoolHeader;

typedef struct GoolState {
    u32 flags;
    u32 status_c;
    u16 extern_idx;
    u16 pc_event;
    u16 pc_trans;
    u16 pc_code;
} GoolState;

typedef struct GoolExecutable {
    u8 reserved_00[0x10];
    RAM_REF(GoolHeader) header;
    RAM_REF(u32) code;
    RAM_REF(u32) externs;
    RAM_REF(u32) events;
    RAM_REF(GoolState) states;
} GoolExecutable;

typedef struct GoolObj {
    s32 header;
    RAM_REF(void) process;
    s32 bound_x1;
    s32 bound_y1;
    s32 bound_z1;
    s32 bound_x2;
    s32 bound_y2;
    s32 bound_z2;
    RAM_REF(GoolExecutable) global;
    RAM_REF(GoolExecutable) external;
    RAM_REF(void) zone;
    s32 state_idx;
    u8 reserved_30[0x60 - 0x30];
    s32 regs[14];
    s32 scale_x;
    s32 scale_y;
    s32 scale_z;
    u8 reserved_a4[0xC8 - 0xA4];
    u32 status_a;
    u32 status_b;
    u32 status_c;
    s32 subtype;
    s32 pid_flags;
    RAM_REF(s32) sp;
    RAM_REF(u32) pc;
    RAM_REF(s32) fp;
    RAM_REF(u32) tp;
    RAM_REF(u32) ep;
    u32 once_p;
    u32 misc_flag;
    RAM_REF(void) entity;
    s32 path_progress;
    s32 state_stamp;
    u32 anim_counter;
    RAM_REF(GoolAnim) anim_seq;
    u32 anim_frame;
    u8 reserved_110[0x120 - 0x110];
    u32 state_flags;
} GoolObj;

void Gool_ObjectLocalBound(GoolObj *arg0, s32 *arg1)
{
    s32 bounds[6];
    GoolAnim *anim;
    u32 *res;
    SvtxFrame *model;
    s32 scale_x;
    s32 *box;
    s32 abs_x;
    s32 val;

    anim = RAM_AT(GoolAnim, arg0->anim_seq);
    if (anim == NULL) {
        return;
    }
    if (anim->type != 4) {
        if (((Ram_Addr(&anim->eid) & 0xFF800003) != 0x80000000) || ((s32)Ns_Lookup(&anim->eid) >= -0xFE)) {
            return;
        }
    }
    if (anim->type == 1) {
        res = RAM_AT(u32, Ns_Lookup(&anim->eid));
        scale_x = arg1[0];
        model = RAM_AT(SvtxFrame, res[(arg0->anim_frame >> 8) + 4]);
        if (scale_x < 0) {
            arg1[0] = -scale_x;
        }
        if (Ram_Addr(arg0) == gp_crash_obj) {
            bounds[0] = model->col_x + model->box[0];
            bounds[1] = model->col_y + model->box[1];
            bounds[2] = model->col_z + model->box[2];
            bounds[3] = model->col_x + model->box[3];
            bounds[4] = model->col_y + model->box[4];
            bounds[5] = model->col_z + model->box[5];
            box = bounds;
        } else {
            box = model->box;
        }
        arg0->bound_x1 = ((box[0] >> 8) * arg1[0]) >> 4;
        arg0->bound_y1 = ((box[1] >> 8) * arg1[1]) >> 4;
        arg0->bound_z1 = ((box[2] >> 8) * arg1[2]) >> 4;
        arg0->bound_x2 = ((box[3] >> 8) * arg1[0]) >> 4;
        arg0->bound_y2 = ((box[4] >> 8) * arg1[1]) >> 4;
        arg0->bound_z2 = ((box[5] >> 8) * arg1[2]) >> 4;
        arg1[0] = scale_x;
    } else {
        abs_x = arg1[0] >= 0 ? arg1[0] : -arg1[0];
        val = (abs_x * 200) >> 4;
        arg0->bound_x2 = val;
        arg0->bound_x1 = -val;
        val = (arg1[1] * 200) >> 4;
        arg0->bound_y2 = val;
        arg0->bound_y1 = -val;
        val = (arg1[2] * 200) >> 4;
        arg0->bound_z2 = val;
        arg0->bound_z1 = -val;
    }
    arg0->scale_x = arg1[0];
    arg0->scale_y = arg1[1];
    arg0->scale_z = arg1[2];
}

typedef struct GoolDbView {
    u8 reserved[0x7C];
    u32 stamp;
} GoolDbView;

#define CRASH_EXT_GOOL_CHANGE_STATE_1
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_CHANGE_STATE_1

typedef struct GoolUpdateGlobals {
    s32 db8[16];
    s32 df8;
    RAM_REF(u8) dfc;
    RAM_REF(void) e00;
    s32 e04;
} GoolUpdateGlobals;

#define CRASH_EXT_GOOL_CHANGE_STATE_2
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_CHANGE_STATE_2

#define GOOL_PUSH(obj, value) (*RAM_AT(s32, (obj)->sp) = (value), (obj)->sp += 4)

/* Visitor form of GOOL_UpdateObject; context points to the flag. */
static s32 Gool_UpdateObjectVisitor(struct Node *node, void *context)
{
    return GOOL_UpdateObject((GoolObj *)node, *(s32 *)context);
}

void Gool_UpdateObjects(s32 arg0)
{
    s32 i;
    s32 *ptr;

    g_gool_update_clear_flag = 0;
    g_percent_complete = g_draw_stamp[0];
    gool_update.e04 = RAM_AT(GoolDbView, g_db_prev[0])->stamp / 34;
    if (gp_crash_obj != 0) {
        i = 0;
        ptr = gool_update.db8;
        do {
            Gool_ObjectHandleTraverseTreePreorder((struct Node *)ptr, Gool_UpdateObjectVisitor, &arg0);
            i++;
            ptr += 2;
        } while (i < 8);
    }
}

/* Visit every live object in the update lists, in update order. */
void Gool_ForEachObject(s32 (*visitor)(struct Node *, void *), void *context)
{
    s32 i;
    s32 *ptr = gool_update.db8;

    if (gp_crash_obj == 0) {
        return;
    }
    for (i = 0; i < 8; i++, ptr += 2) {
        Gool_ObjectHandleTraverseTreePreorder((struct Node *)ptr, visitor, context);
    }
}

s32 GOOL_ChangeObjectState(GoolObj *arg0, s32 arg1, s32 arg2, s32 *arg3)
{
    u32 saved_f0;
    GoolExecutable *global;
    GoolState *state_desc;
    s32 i;

    saved_f0 = arg0->once_p;
    if (arg1 == 0xFF || (arg0->status_b & 0x10000000)) {
        return -0xE;
    }
    arg0->state_idx = arg1;
    global = RAM_AT(GoolExecutable, arg0->global);
    state_desc = &RAM_AT(GoolState, global->states)[arg1];
    arg0->status_c = state_desc->status_c;
    arg0->external = Ns_Lookup(&RAM_AT(u32, global->externs)[state_desc->extern_idx]);
    arg0->pc = (state_desc->pc_code != 0x3FFF)
        ? RAM_AT(GoolExecutable, arg0->external)->code + state_desc->pc_code * 4
        : 0;
    arg0->ep = (state_desc->pc_event != 0x3FFF)
        ? RAM_AT(GoolExecutable, arg0->external)->code + state_desc->pc_event * 4
        : 0;
    arg0->tp = (state_desc->pc_trans != 0x3FFF)
        ? RAM_AT(GoolExecutable, arg0->external)->code + state_desc->pc_trans * 4
        : 0;
    arg0->once_p = 0;
    arg0->fp = Ram_Addr(arg0->regs);
    arg0->sp = Ram_Addr(&arg0->regs[RAM_AT(GoolHeader, global->header)->sp_offset]);
    arg0->state_flags = state_desc->flags;
    arg0->status_a |= 0x20020;
    for (i = 0; i < arg2; i++) {
        GOOL_PUSH(arg0, arg3[i]);
    }
    GOOL_CreateObjectStackFrame((struct GoolRuntimeObject *)arg0, arg2, 0xFFFF);
    if (saved_f0 != 0) {
        GOOL_CreateObjectStackFrame((struct GoolRuntimeObject *)arg0, 0, 0xFFFF);
        arg0->pc = saved_f0;
        i = GOOL_InterpretObject(arg0, 0x13, 0);
        if (i >= -0xFE) {
            return i;
        }
    } else {
        GOOL_PUSH(arg0, 0);
    }
    arg0->state_stamp = g_level_zone_flags[0];
    if (arg0->tp != 0 && Ram_Addr(arg0) == g_savestate[0]) {
        GOOL_CreateObjectStackFrame((struct GoolRuntimeObject *)arg0, 0, 0xFFFF);
        arg0->pc = arg0->tp;
        i = GOOL_InterpretObject(arg0, 3, 0);
        if (i >= -0xFE) {
            return i;
        }
    }
    return -0xFF;
}
