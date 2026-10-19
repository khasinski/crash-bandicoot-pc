#include "common.h"

static __inline__ s32 Gool_SignShift(s32 value, s32 bits)
{
    s32 shift = 0x20 - bits;
    return (s32)((u32)value << shift) >> shift;
}

typedef struct GoolExecView {
    u8 reserved_00[0x10];
    struct {
        s32 word_00;
        s32 word_04;
        s32 word_08;
        s32 word_0c;
        s32 word_10;
    } *header;
    RAM_REF(u8) code;
    RAM_REF(s32) data;
    RAM_REF(u16) states;
    RAM_REF(u8) word_20;
} GoolExecView;

typedef struct GoolObjView {
    u8 reserved_00[0x20];
    RAM_REF(GoolExecView) exec;
    RAM_REF(GoolExecView) external;
    u8 reserved_28[0x38];
    RAM_REF(struct GoolObjView) links[8];
    u8 reserved_80[0x48];
    u32 status_a;
    u32 status_b;
    u32 status_c;
    u8 reserved_d4[0x8];
    RAM_REF(s32) sp;
    RAM_REF(u32) pc;
    RAM_REF(s32) fp;
    RAM_REF(u32) tp;
    RAM_REF(u32) ep;
    s32 result;
    s32 misc_flag;
    u8 reserved_f8[0x30];
    s32 anim_type;
    u8 reserved_12c[0x10];
    s32 event;
} GoolObjView;

typedef struct GoolTimeState {
    u8 reserved_00[0x84];
    u32 word_84;
} GoolTimeState;

#define CRASH_EXT_GOOL_VM_OPERANDS_0
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_VM_OPERANDS_0

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define SCALE_DT(x) (((s32)((x) * (g_db_cur[0]->word_84 < 0x67U ? g_db_cur[0]->word_84 : 0x66))) / 1024)

static __inline__ s32 *GoolRegPtr(GoolObjView *obj, u32 reg, s32 mode)
{
    if (reg == 0x1F) {
        if (mode == 0) {
            return --obj->sp;
        } else {
            return obj->sp++;
        }
    } else {
        return (s32 *)&obj->links[reg];
    }
}

s32 *Gool_TranslateOutGop(GoolObjView *obj, u32 gop, s32 mode)
{
    GoolObjView *link;
    u32 offset;
    GoolExecView *ptr;
    u32 idx;

    if (gop & 0x800) {
        if (gop & 0x400) {
            if (gop & 0x200) {
                return GoolRegPtr(obj, gop & 0x1FF, mode);
            } else {
                link = obj->links[(gop >> 6) & 7];
                if (link != NULL) {
                    offset = ((gop & 0x3F) << 2) + 0x60;
                    return (s32 *)(offset + (u32)link);
                }
                return NULL;
            }
        } else if (gop & 0x200) {
            if (gop & 0x100) {
                if (gop & 0x80) {
                    if (gop == 0xBE0) {
                        return NULL;
                    } else if (gop == 0xBF0) {
                        return (s32 *)1;
                    } else if (0) {
                        printf("");
                    }
                } else {
                    return &obj->fp[Gool_SignShift(gop, 7)];
                }
            } else {
                ptr = (GoolExecView *)&gp_obj_zone_colors[gp_gool_out_vec_toggle = !gp_gool_out_vec_toggle];
                *(s32 *)ptr = Gool_SignShift(gop, 8) << 4;
                return (s32 *)ptr;
            }
        } else {
            ptr = (GoolExecView *)&gp_obj_zone_colors[gp_gool_out_vec_toggle = !gp_gool_out_vec_toggle];
            *(s32 *)ptr = Gool_SignShift(gop, 9) << 8;
            return (s32 *)ptr;
        }
    } else {
        idx = gop & 0x3FF;
        if (gop & 0x400) {
            ptr = obj->external;
        } else {
            ptr = obj->exec;
        }
        return (s32 *)((idx <<= 2) + (u32)(ptr = (GoolExecView *)ptr->data));
    }
}

typedef struct GoolPad {
    u32 word_0;
    u32 word_4;
    u32 word_8;
    u32 word_c;
    u32 word_10;
} GoolPad;

typedef struct GoolDirEntry {
    s32 dir;
    s32 angle;
    s32 speed_scale;
} GoolDirEntry;

#define CRASH_EXT_GOOL_VM_OPERANDS_1
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_VM_OPERANDS_1

s32 Gool_TestControls(u32 arg0, s32 port)
{
    u32 res;
    s32 dir_test;

    res = 1;
    switch ((arg0 >> 12) & 3) {
    case 1:
        res = g_gool_pads[port].word_0 & (arg0 & 0xFFF);
        break;
    case 2:
        res = g_gool_pads[port].word_4 & (arg0 & 0xFFF);
        break;
    case 4:
        dir_test = arg0 & 0xFFF;
        res = dir_test & (g_gool_pads[port].word_4 | g_gool_pads[port].word_8);
        break;
    case 3:
        dir_test = arg0 & 0xFFF;
        res = dir_test & (g_gool_pads[port].word_0 | g_gool_pads[port].word_c);
        break;
    }
    if (res == 0) {
        return (arg0 >> 20) & 1;
    }
    dir_test = (arg0 >> 16) & 0xF;
    switch ((arg0 >> 14) & 3) {
    case 1:
        switch (dir_test) {
        case 9:
            res = g_gool_pads[port].word_0 & 0x1000;
            break;
        case 10:
            res = g_gool_pads[port].word_0 & 0x2000;
            break;
        case 11:
            res = g_gool_pads[port].word_0 & 0x4000;
            break;
        case 12:
            res = g_gool_pads[port].word_0 & 0x8000;
            break;
        default:
            res = (dir_test == g_move_states[(g_gool_pads[port].word_4 & 0xF000) >> 12].dir)
               && (dir_test != g_move_states[(g_gool_pads[port].word_8 & 0xF000) >> 12].dir);
            break;
        }
        break;
    case 2:
        switch (dir_test) {
        case 9:
            res = g_gool_pads[port].word_4 & 0x1000;
            break;
        case 10:
            res = g_gool_pads[port].word_4 & 0x2000;
            break;
        case 11:
            res = g_gool_pads[port].word_4 & 0x4000;
            break;
        case 12:
            res = g_gool_pads[port].word_4 & 0x8000;
            break;
        default:
            res = (dir_test == g_move_states[(g_gool_pads[port].word_4 & 0xF000) >> 12].dir);
            break;
        }
        break;
    case 3:
    case 4:
        res = ((dir_test = (arg0 >> 16) & 0xF) == g_move_states[(g_gool_pads[port].word_4 & 0xF000) >> 12].dir)
           || (dir_test == g_move_states[(g_gool_pads[port].word_8 & 0xF000) >> 12].dir);
        break;
    }
    dir_test = res;
    if (arg0 & 0x100000) {
        dir_test = !dir_test;
    }
    return dir_test;
}
