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

typedef struct GoolTimeState {
    u8 reserved_00[0x84];
    u32 word_84;
} GoolTimeState;

#define CRASH_EXT_GOOL_SEND_EVENT_0
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_SEND_EVENT_0

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define SCALE_DT(x) (((s32)((x) * (g_db_cur[0]->word_84 < 0x67U ? g_db_cur[0]->word_84 : 0x66))) / 1024)

#define CRASH_EXT_GOOL_SEND_EVENT_1
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_SEND_EVENT_1

static __inline__ s32 GOOL_CreateObjectStackFrame_inline(GoolObjView *object, s32 arg_words, s32 flags)
{
    u32 *sp;
    u32 *old_fp;
    u32 *new_sp;
    u32 *new_fp;

    sp = (u32 *)object->sp;
    old_fp = (u32 *)object->fp;
    object->fp = (s32 *)sp;
    *object->sp++ = flags;
    *object->sp++ = (s32)object->pc;
    *object->sp++ = ((u32)(new_sp = sp - arg_words - 24) - (u32)object) | (((u32)(new_fp = old_fp - 24) - (u32)object) << 16);
    return -0xFF;
}

s32 Gool_ExecStateCode(GoolObjView *sender, GoolObjView *recipient, s32 event, s32 argc, s32 *argv)
{
    s32 sp10[2];
    s32 res;
    u32 state;
    s32 idx;
    s32 i;
    u32 status_c;
    u32 target_flags;
    s32 anim_type;
    u32 test;

    res = -0x1A;
    if (recipient != NULL) {
        if (sender != NULL) {
            sender->misc_flag = 1;
        }
        recipient->links[7] = sender;
        if (recipient->ep != NULL) {
            *recipient->sp++ = event;
            *recipient->sp++ = (s32)argv;
            GOOL_CreateObjectStackFrame_inline(recipient, 2, 0xFFFF);
            recipient->pc = recipient->ep;
            res = GOOL_InterpretObject(recipient, 8, sp10);
            if (res != -0x1A) {
                if (res >= -0xFE) {
                    return res;
                }
                state = sp10[0];
            }
            if (sender != NULL) {
                sender->misc_flag = sp10[1];
            }
        }
        if (res >= -0xFE) {
            idx = event >> 8;
            if (idx < recipient->exec->header->word_10) {
                state = recipient->exec->states[idx];
            } else {
                state = 0xFF;
            }
            if (sender != NULL) {
                sender->misc_flag = (state != 0xFF);
            }
            if (state & 0x8000) {
                recipient->event = event;
                for (i = 0; i < argc; i++) {
                    *recipient->sp++ = argv[i];
                }
                GOOL_CreateObjectStackFrame_inline(recipient, argc, 0xFFFF);
                recipient->pc = (u32 *)recipient->exec->code + (state & 0x7FFF);
                return GOOL_InterpretObject(recipient, 3, 0);
            }
        }
        if (state != 0xFF) {
            i = recipient->status_c;
            if (event == 0x1800 || event == 0x1900 || event == 0x2500) {
                i &= ~2;
            }
            target_flags = *(u32 *)((state << 4) + (u32)recipient->exec->word_20);
            if (recipient == gp_crash_obj) {
                anim_type = recipient->anim_type;
                if ((u32)(anim_type - 3) < 2 || anim_type == 2) {
                    test = (i | 0x1002) & target_flags;
                } else {
                    test = i & target_flags;
                }
            } else {
                test = i & target_flags;
            }
            if (test == 0) {
                recipient->event = event;
                if (event == 0x1800 || event == 0x1900) {
                    recipient->status_a |= 0x10000;
                }
                GOOL_ChangeObjectState(recipient, state, argc, argv);
            } else if (state != 0xFF && sender != NULL) {
                sender->misc_flag = 0;
            }
        }
    } else if (sender != NULL) {
        sender->misc_flag = 0;
    }
    return -0xFF;
}

s32 Gool_ObjectRotate(s32 cur_ang, s32 tgt_ang, s32 speed, GoolObjView *obj)
{
    s32 diff;
    s32 wrapped_diff;

    speed = SCALE_DT(speed);
    cur_ang &= 0xFFF;
    tgt_ang &= 0xFFF;
    diff = tgt_ang - cur_ang;
    if (ABS(diff) >= 0x801) {
        if (diff > 0) {
            wrapped_diff = diff - 0x1000;
        } else {
            wrapped_diff = diff + 0x1000;
        }
    } else {
        wrapped_diff = diff;
    }
    diff = ABS(wrapped_diff);
    if (obj != NULL && (obj->status_b & 0x20000000) && diff < (speed << 2)) {
        if (diff < (speed << 1)) {
            if (diff < speed) {
                speed /= 8;
            } else {
                speed /= 4;
            }
        } else if (diff < speed * 3) {
            speed /= 2;
        } else {
            speed = (speed * 3) / 4;
        }
    }
    if (diff < speed) {
        cur_ang = tgt_ang;
        if (obj != NULL) {
            obj->status_a |= 0x800;
        }
    } else {
        if (ABS(diff) == 0x800 && tgt_ang >= 0x800) {
            wrapped_diff = -wrapped_diff;
        }
        if (wrapped_diff < 0) {
            cur_ang = (cur_ang - speed) & 0xFFF;
            if (obj != NULL) {
                obj->status_a |= 8;
            }
        } else {
            cur_ang = (cur_ang + speed) & 0xFFF;
            if (obj != NULL) {
                obj->status_a &= ~8;
            }
        }
    }
    return cur_ang;
}

s32 Gool_ObjectRotate2(s32 cur_ang, s32 tgt_ang, s32 speed, GoolObjView *obj)
{
    s32 step;
    s32 diff;

    step = SCALE_DT(speed);
    cur_ang &= 0xFFF;
    tgt_ang &= 0xFFF;
    diff = tgt_ang - cur_ang;
    if (ABS(diff) >= 0x801) {
        if (diff > 0) {
            speed = diff - 0x1000;
        } else {
            speed = diff + 0x1000;
        }
    } else {
        speed = diff;
    }
    if (speed == 0 || ((diff = ABS(speed)) < ABS(step) && (step ^ speed) >= 0)) {
        cur_ang = tgt_ang;
        if (obj != NULL) {
            obj->status_a |= 0x800;
        }
    } else {
        cur_ang = (cur_ang + step) & 0xFFF;
    }
    return cur_ang;
}
