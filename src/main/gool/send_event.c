#include "common.h"

typedef struct GoolExecHeader {
    s32 word_00;
    s32 word_04;
    s32 word_08;
    s32 word_0c;
    s32 word_10;
} GoolExecHeader;

typedef struct GoolExecView {
    u8 reserved_00[0x10];
    RAM_REF(GoolExecHeader) header;
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

#define CRASH_EXT_GOOL_SEND_EVENT_0
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_SEND_EVENT_0

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define SCALE_DT(x) (((s32)((x) * (RAM_AT(GoolTimeState, g_db_cur[0])->word_84 < 0x67U ? RAM_AT(GoolTimeState, g_db_cur[0])->word_84 : 0x66))) / 1024)

#define CRASH_EXT_GOOL_SEND_EVENT_1
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_SEND_EVENT_1

#define GOOL_PUSH(obj, value) (*RAM_AT(s32, (obj)->sp) = (value), (obj)->sp += 4)

static __inline__ s32 Gool_InGameMemory(const void *p)
{
    return (uintptr_t)((const u8 *)p - g_ps1_ram) < RAM_SIZE ||
           (uintptr_t)((const u8 *)p - g_ps1_scratch) < RAM_SCRATCH_SIZE;
}

s32 Gool_ExecStateCode(GoolObjView *sender, GoolObjView *recipient, s32 event, s32 argc, s32 *argv)
{
    s32 sp10[2];
    s32 res;
    u32 state;
    s32 idx;
    s32 i;
    u32 target_flags;
    s32 anim_type;
    u32 test;
    GoolExecView *exec;

    res = -0x1A;
    if (recipient != NULL) {
        if (sender != NULL) {
            sender->misc_flag = 1;
        }
        recipient->links[7] = Ram_Addr(sender);
        if (recipient->ep != 0) {
            u32 argv_addr = 0;
            u32 copy_size = 0;

            /* The handler reads the arguments through a PS1 address. Arguments
             * in host memory are copied to the RAM stack for the call. */
            if (argv == NULL || Gool_InGameMemory(argv)) {
                argv_addr = Ram_Addr(argv);
            } else if (argc > 0) {
                copy_size = argc * 4;
                argv_addr = Ram_StackAlloc(copy_size);
                memcpy(Ram_Ptr(argv_addr), argv, copy_size);
            }
            GOOL_PUSH(recipient, event);
            GOOL_PUSH(recipient, argv_addr);
            GOOL_CreateObjectStackFrame((struct GoolRuntimeObject *)recipient, 2, 0xFFFF);
            recipient->pc = recipient->ep;
            res = GOOL_InterpretObject((struct GoolObj *)recipient, 8, (struct GoolStateOut *)sp10);
            if (copy_size != 0) {
                Ram_StackFree(copy_size);
            }
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
        exec = RAM_DEREF(GoolExecView, recipient->exec); /* 0 reads low RAM, as on the PS1 */
        if (res >= -0xFE) {
            idx = event >> 8;
            if (idx < RAM_DEREF(GoolExecHeader, exec->header)->word_10) {
                state = RAM_AT(u16, exec->states)[idx];
            } else {
                state = 0xFF;
            }
            if (sender != NULL) {
                sender->misc_flag = (state != 0xFF);
            }
            if (state & 0x8000) {
                recipient->event = event;
                for (i = 0; i < argc; i++) {
                    GOOL_PUSH(recipient, argv[i]);
                }
                GOOL_CreateObjectStackFrame((struct GoolRuntimeObject *)recipient, argc, 0xFFFF);
                recipient->pc = exec->code + (state & 0x7FFF) * 4;
                return GOOL_InterpretObject((struct GoolObj *)recipient, 3, 0);
            }
        }
        if (state != 0xFF) {
            i = recipient->status_c;
            if (event == 0x1800 || event == 0x1900 || event == 0x2500) {
                i &= ~2;
            }
            target_flags = *RAM_AT(u32, (state << 4) + exec->word_20);
            if (Ram_Addr(recipient) == gp_crash_obj) {
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
                GOOL_ChangeObjectState((struct GoolObj *)recipient, state, argc, argv);
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
