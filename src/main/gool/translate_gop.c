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
    u8 *code;
    s32 *data;
    u16 *states;
    u8 *word_20;
} GoolExecView;

typedef struct GoolObjView {
    u8 reserved_00[0x20];
    GoolExecView *exec;
    GoolExecView *external;
    u8 reserved_28[0x38];
    struct GoolObjView *links[8];
    u8 reserved_80[0x48];
    u32 status_a;
    u32 status_b;
    u32 status_c;
    u8 reserved_d4[0x8];
    s32 *sp;
    u32 *pc;
    s32 *fp;
    u32 *tp;
    u32 *ep;
    s32 result;
    s32 misc_flag;
    u8 reserved_f8[0x30];
    s32 anim_type;
    u8 reserved_12c[0x10];
    s32 event;
} GoolObjView;

s32 *GOOL_TranslateGop(GoolObjView *obj, u32 gop)
{
    GoolObjView *link;
    u32 offset;
    GoolExecView *ptr;
    u32 idx;

    if (gop & 0x800) {
        if (gop & 0x400) {
            link = obj->links[(gop >> 6) & 7];
            if (link != NULL) {
                offset = ((gop & 0x3F) << 2) + 0x60;
                return (s32 *)(offset + (u32)link);
            }
            return NULL;
        } else if (gop & 0x200) {
            if (gop & 0x100) {
                if (gop & 0x80) {
                    if (gop == 0xBE0) {
                        return NULL;
                    } else if (gop == 0xBF0) {
                        return (s32 *)1;
                    } else if (0) {
                        printf(0);
                    }
                } else {
                    return &obj->fp[Gool_SignShift(gop, 7)];
                }
            } else {
                ptr = (GoolExecView *)&gp_crash[gp_gool_in_vec_toggle = !gp_gool_in_vec_toggle];
                *(s32 *)ptr = Gool_SignShift(gop, 8) << 4;
                return (s32 *)ptr;
            }
        } else {
            ptr = (GoolExecView *)&gp_crash[gp_gool_in_vec_toggle = !gp_gool_in_vec_toggle];
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
