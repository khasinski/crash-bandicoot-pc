#include "common.h"

#define CRASH_EXT_GOOL_QUERY_NEAREST
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_QUERY_NEAREST

typedef struct GoolCodeHeader {
    s32 word_00;
    s32 category;
} GoolCodeHeader;

typedef struct GoolExecutable {
    u8 reserved_00[0x10];
    RAM_REF(GoolCodeHeader) header;
    RAM_REF(void) code;
    RAM_REF(void) data;
    RAM_REF(u16) states;
    s32 states_base;
} GoolExecutable;

typedef struct GoolObj {
    u8 reserved_00[0x20];
    RAM_REF(GoolExecutable) exec;
    u8 reserved_24[0x58];
    RAM_REF(struct GoolObj) interrupter;
    s32 trans[3];
    u8 reserved_8c[0x44];
    u32 status_c;
    u8 reserved_d4[0x24];
    s32 arg0_v;
    u8 reserved_fc[0x24];
    u32 state_flags;
    u8 reserved_124[4];
    s32 anim_type;
} GoolObj;

typedef struct GoolQuery {
    u32 mask;
    RAM_REF(GoolObj) other;
    RAM_REF(GoolObj) best;
    s32 max_dist;
    s32 event;
} GoolQuery;

s32 Gool_FindNearestObject(GoolObj *arg0, GoolQuery *arg1)
{
    GoolObj *other;
    s32 dist;
    s32 event;
    GoolExecutable *hdr;
    u32 state;
    u32 state_flags1;
    u32 state_flags2;
    s32 anim_type2;
    u32 target_flags;
    u32 test;
    u32 sp10;

    other = RAM_AT(GoolObj, arg1->other);
    if (arg0 == other) {
        return 0;
    }
    if (!((1 << (RAM_AT(GoolCodeHeader, RAM_AT(GoolExecutable, arg0->exec)->header)->category >> 8)) & arg1->mask)) {
        return 0;
    }
    dist = Math_ApxDist((struct Vec3 *)arg0->trans, (struct Vec3 *)other->trans);
    if (dist < arg1->max_dist) {
        event = arg1->event;
        if (event != 0xFF) {
            hdr = RAM_AT(GoolExecutable, arg0->exec);
            state = RAM_AT(u16, hdr->states)[event >> 8];
            if (state == 0xFF) {
                if ((event == 0x300 && (event = arg0->anim_type, state_flags1 = arg0->status_c, (u32)(event - 3) >= 2 && event != 2 && !(state_flags1 & 2)))
                    || (arg1->event == 0xA00 && !(arg0->state_flags & 0x800))
                    || arg1->event == 0x1D00) {
                    state = 0xFE;
                } else {
                    return 0;
                }
            } else {
                if (!(state & 0x8000)) {
                    state_flags2 = arg0->status_c;
                    anim_type2 = arg0->anim_type;
                    target_flags = *RAM_AT(u32, (state << 4) + hdr->states_base);
                    if ((u32)(anim_type2 - 3) < 2 || anim_type2 == 2) {
                        test = (state_flags2 | 0x1002) & target_flags;
                    } else {
                        test = state_flags2 & target_flags;
                    }
                    if (test != 0) {
                        return 0;
                    }
                }
                if (state & 0x8000) {
                    if (arg1->event == 0xF00) {
                        sp10 = 0x100;
                        arg0->interrupter = arg1->other;
                        Gool_ObjectInterrupt((struct GoolStackObj *)arg0, state, 1, &sp10);
                        if (RAM_AT(GoolObj, arg1->other)->arg0_v == 0) {
                            return 0;
                        }
                    }
                }
            }
        } else {
            state = 0xFE;
        }
        if (state != 0xFF) {
            arg1->best = Ram_Addr(arg0);
            arg1->max_dist = dist;
        }
    }
    return 0;
}
