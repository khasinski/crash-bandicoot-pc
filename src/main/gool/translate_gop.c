#include "common.h"

typedef struct GoolExecView {
    u8 reserved_00[0x10];
    RAM_REF(void) header;
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

/* PS1 address of a GOOL input operand: an object field, a frame or data
 * word, or a scratch word holding an immediate. 0 and 1 are sentinels. */
u32 GOOL_TranslateGop(GoolObjView *obj, u32 gop)
{
    u32 link;
    u32 ptr;

    if (gop & 0x800) {
        if (gop & 0x400) {
            link = obj->links[(gop >> 6) & 7];
            if (link != 0) {
                return link + ((gop & 0x3F) << 2) + 0x60;
            }
            return 0;
        } else if (gop & 0x200) {
            if (gop & 0x100) {
                if (gop & 0x80) {
                    if (gop == 0xBE0) {
                        return 0;
                    } else if (gop == 0xBF0) {
                        return 1;
                    }
                    /* The original falls through with 0xBF0 left in v0. */
                    return 0xBF0;
                } else {
                    return obj->fp + Gool_SignShift(gop, 7) * 4;
                }
            } else {
                ptr = gp_crash + ((gp_gool_in_vec_toggle = !gp_gool_in_vec_toggle) << 2);
                *RAM_AT(s32, ptr) = Gool_SignShift(gop, 8) * 0x10;
                return ptr;
            }
        } else {
            ptr = gp_crash + ((gp_gool_in_vec_toggle = !gp_gool_in_vec_toggle) << 2);
            *RAM_AT(s32, ptr) = Gool_SignShift(gop, 9) * 0x100;
            return ptr;
        }
    } else {
        if (gop & 0x400) {
            ptr = obj->external;
        } else {
            ptr = obj->exec;
        }
        return RAM_AT(GoolExecView, ptr)->data + ((gop & 0x3FF) << 2);
    }
}
