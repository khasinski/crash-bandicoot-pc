#include "common.h"

typedef struct Vec3 {
    s32 x;
    s32 y;
    s32 z;
} Vec3;

typedef struct GoolBound {
    Vec3 p1;
    Vec3 p2;
} GoolBound;

typedef struct GoolVectors {
    Vec3 trans;
    Vec3 rot;
    Vec3 scale;
} GoolVectors;

typedef struct GoolCodeHeader {
    s32 word_00;
    s32 word_04;
    s32 word_08;
    s32 word_0c;
    s32 word_10;
} GoolCodeHeader;

typedef struct GoolState {
    u32 status_c;
    u32 flags;
    u16 extern_idx;
    u16 pc_event;
    u16 pc_trans;
    u16 pc_code;
} GoolState;

typedef struct GoolExecutable {
    u8 reserved_00[0x10];
    RAM_REF(GoolCodeHeader) header;
    RAM_REF(u32) code;
    RAM_REF(u32) data;
    RAM_REF(void) word_1c;
    RAM_REF(GoolState) states;
    RAM_REF(u8) anims;
} GoolExecutable;

typedef struct GoolDefaults {
    u8 bytes[0x2A];
    u16 intensity_r, intensity_g, intensity_b;
} GoolDefaults;

typedef struct ZoneHeaderView {
    u8 reserved_00[0x210];
    s32 neighbor_count;
    u32 neighbors[8];
    u8 reserved_234[0xC8];
    u32 flags2FC;
} ZoneHeaderView;

typedef struct ResourceEntry {
    u32 word00;
    u32 id;
    s32 kind;
    s32 word0C;
    RAM_REF(ZoneHeaderView) data;
} ResourceEntry;

typedef struct GoolAnimVertex {
    u8 x;
    u8 y;
    u8 z;
    u8 reserved[3];
} GoolAnimVertex;

typedef struct GoolAnimFrame {
    u8 reserved_00[8];
    Vec3 offset;
    u8 reserved_14[0x24];
    GoolAnimVertex verts[1];
} GoolAnimFrame;

typedef struct GoolAnimModel {
    u8 reserved_00[4];
    Vec3 scale;
} GoolAnimModel;

typedef struct GoolAnimEntry {
    u8 reserved_00[0x10];
    RAM_REF(void) items[1];
} GoolAnimEntry;

typedef struct GoolAnimSeq {
    u8 type;
    u8 reserved_01[3];
    u32 eid;
    u32 model_eid;
} GoolAnimSeq;

typedef struct GoolObj {
    s32 kind;
    s32 word_04;
    GoolBound bound;
    RAM_REF(const GoolExecutable) exec;
    RAM_REF(GoolExecutable) external;
    RAM_REF(ResourceEntry) zone;
    s32 word_2c;
    u16 defaults[24];
    RAM_REF(struct GoolObj) self;
    RAM_REF(struct GoolObj) parent;
    RAM_REF(struct GoolObj) next;
    RAM_REF(struct GoolObj) children;
    RAM_REF(struct GoolObj) creator;
    RAM_REF(struct GoolObj) player;
    RAM_REF(struct GoolObj) collider;
    RAM_REF(struct GoolObj) interrupter;
    Vec3 trans;
    Vec3 rot;
    Vec3 scale;
    Vec3 vel;
    Vec3 ang_vel;
    Vec3 target_rot;
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
    s32 result_v;
    s32 misc_flag;
    s32 arg0;
    s32 arg1;
    s32 arg2;
    s32 arg3_v;
    RAM_REF(GoolAnimSeq) anim_seq;
    s32 anim_frame;
    RAM_REF(void) entity;
    s32 path_progress;
    s32 path_length;
    s32 floor_y;
    u32 state_flags;
    s32 speed;
    s32 invincibility_state;
    s32 invincibility_time;
    s32 floor_time;
    s32 floor_vel_y;
    s32 cam_zoom;
    s32 event;
    s32 cam_rot_x;
    s32 ang_vel_x;
    s32 head_turn;
    s32 voice_id;
    s32 ack_proj;
    s32 hotspot_size;
    s32 node_id;
} GoolObj;

typedef struct GoolStateOut {
    s32 state;
    s32 flag;
} GoolStateOut;

typedef struct GoolQuery {
    s32 kind;
    RAM_REF(GoolObj) obj;
    RAM_REF(GoolObj) result;
    s32 best_dist;
    s32 max_dist;
} GoolQuery;

typedef struct ObjectRoot {
    s32 kind;
    RAM_REF(GoolObj) word04;
} ObjectRoot;

typedef struct LevelStateView {
    u8 reserved_00[4];
    u16 level_id;
} LevelStateView;

typedef struct PadStateView {
    u8 reserved_00[0x84];
    u32 word_84;
} PadStateView;

#define CRASH_EXT_GOOL_INTERPRET
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_INTERPRET

typedef s32 (*Visitor)(struct Node *, void *);

#define ABS(x) ((x) < 0 ? -(x) : (x))
#define SCALE_DT(x) (((s32)((x) * (RAM_AT(PadStateView, g_db_cur[0])->word_84 < 0x67U ? RAM_AT(PadStateView, g_db_cur[0])->word_84 : 0x66))) / 1024)

/* Child list head of a tree node (word_04 for a list root of kind 2). */
#define NODE_CHILDREN(n) ((n)->kind == 2 ? (u32)(n)->word_04 : (n)->children)
#define NODE_CHILDREN_PTR(n) ((n)->kind == 2 ? (u32 *)&(n)->word_04 : &(n)->children)

/* Object link i (self, parent, ..., interrupter). */
#define OBJ_LINK(obj, i) RAM_AT(GoolObj, (&(obj)->self)[i])

static __inline__ void GOOL_PUSH(GoolObj *obj, s32 value)
{
    *RAM_AT(s32, obj->sp) = value;
    obj->sp += 4;
}

static __inline__ s32 GOOL_POP(GoolObj *obj)
{
    obj->sp -= 4;
    return *RAM_AT(s32, obj->sp);
}

/* Operand translation yields PS1 addresses, as GOOL code sees them. */
static __inline__ u32 TRANS_GOP_IN_ADDR(GoolObj *obj, s32 gop)
{
    if ((gop & 0xE00) == 0xE00) {
        if (gop == 0xE1F) {
            obj->sp -= 4;
            return obj->sp;
        }
        return Ram_Addr(obj) + ((gop & 0x1FF) << 2) + 0x60;
    }
    return GOOL_TranslateGop((struct GoolObjView *)obj, gop);
}

static __inline__ u32 TRANS_GOP_OUT_ADDR(GoolObj *obj, s32 gop)
{
    u32 sp;

    if ((gop & 0xE00) == 0xE00) {
        if (gop == 0xE1F) {
            sp = obj->sp;
            obj->sp += 4;
            return sp;
        }
        return Ram_Addr(obj) + ((gop & 0x1FF) << 2) + 0x60;
    }
    return GOOL_TranslateGop((struct GoolObjView *)obj, gop);
}

static __inline__ s32 *TRANS_GOP_IN(GoolObj *obj, s32 gop)
{
    return RAM_AT(s32, TRANS_GOP_IN_ADDR(obj, gop));
}

static __inline__ s32 *TRANS_GOP_OUT(GoolObj *obj, s32 gop)
{
    return RAM_AT(s32, TRANS_GOP_OUT_ADDR(obj, gop));
}

static __inline__ s32 *TRANS_REG_IN(GoolObj *obj, s32 reg)
{
    if (reg == 0x1F) {
        obj->sp -= 4;
        return RAM_AT(s32, obj->sp);
    }
    return (s32 *)((u8 *)obj + (reg << 2) + 0x60);
}

static __inline__ s32 *TRANS_REG_OUT(GoolObj *obj, s32 reg)
{
    u32 sp;

    if (reg == 0x1F) {
        sp = obj->sp;
        obj->sp += 4;
        return RAM_AT(s32, sp);
    }
    return (s32 *)((u8 *)obj + (reg << 2) + 0x60);
}

static __inline__ s32 GOOL_CreateObjectStackFrame_inline(GoolObj *object, s32 arg_words, s32 flags)
{
    u32 base;
    u32 sp;
    u32 old_fp;

    base = Ram_Addr(object);
    sp = object->sp;
    old_fp = object->fp;
    object->fp = sp;
    GOOL_PUSH(object, flags);
    GOOL_PUSH(object, object->pc);
    GOOL_PUSH(object, ((sp - (arg_words + 24) * 4) - base) | (((old_fp - 24 * 4) - base) << 16));
    return -0xFF;
}

static __inline__ s32 Gool_ObjectPopFrame(GoolObj *object, s32 *flags, u32 cur_flags)
{
    u32 packed;
    u32 frame_offset;
    u32 frame;
    u32 base;

    base = Ram_Addr(object);
    frame = object->fp;
    object->sp = frame + 8;
    packed = RAM_AT(u32, frame)[2];
    frame_offset = packed >> 16;
    if (frame_offset == 0) {
        return -0x19;
    }
    object->sp = frame + 4;
    object->pc = RAM_AT(u32, frame)[1];
    if (flags != NULL) {
        object->sp -= 4;
        *flags = (cur_flags & 0xFFFF0000) | *RAM_AT(u16, object->sp);
    }
    object->sp = (packed & 0xFFFF) + 0x60 + base;
    object->fp = frame_offset + 0x60 + base;
    return -0xFF;
}

static __inline__ s32 Gool_FindObject_inline(GoolObj *root, Visitor visitor, void *context)
{
    GoolObj *child;
    GoolObj *next;
    s32 result = 0;

    if (root == NULL) {
        return 0;
    }
    if (root->kind != 2) {
        result = visitor((struct Node *)root, context);
        if (result != 0) {
            return result;
        }
    }
    if (root->kind == 2) {
        child = RAM_AT(GoolObj, root->word_04);
    } else {
        child = RAM_AT(GoolObj, root->children);
    }
    while (child != NULL && result == 0) {
        next = RAM_AT(GoolObj, child->next);
        result = Gool_ObjectSearchTree((struct Node *)child, visitor, context);
        child = next;
    }
    return result;
}

/* Visitor adapters: the context is a pointer to the pid, and the found
 * object is returned as its PS1 address. */
static s32 Gool_VisitHasPidFlags(struct Node *node, void *context)
{
    return Ram_Addr(Gool_ObjectHasPidFlags((GoolObj *)node, *(s32 *)context));
}

static s32 Gool_VisitFindNearest(struct Node *node, void *context)
{
    return Gool_FindNearestObject((GoolObj *)node, (GoolQuery *)context);
}

s32 GOOL_InterpretObject(GoolObj *obj, u32 flags, GoolStateOut *out_state)
{
    u32 ins;
    u32 opcode;

    GoolObj *recipient;
    s32 tmp_swap;
    s32 cond1;
    s32 cond2;

next_ins:
    while (1) {
        ins = *RAM_AT(u32, obj->pc);
        obj->pc += 4;
        opcode = ins >> 24;
        switch (opcode) {
        case 0x80:
        case 0x81:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            break;

        }
        case 0x11:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_val = *TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_OUT(obj, ins & 0xFFF);
            if (d_ptr != NULL) {
                *d_ptr = s_val;
            }
            break;

        }
        case 0x16:
        case 0x26:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            u32 s_addr; u32 d_addr;
            s_addr = TRANS_GOP_IN_ADDR(obj, (ins >> 12) & 0xFFF);
            d_addr = TRANS_GOP_IN_ADDR(obj, ins & 0xFFF);
            s_ptr = RAM_AT(s32, s_addr);
            d_ptr = RAM_AT(s32, d_addr);
            if ((ins >> 24) == 0x26) {
                if (d_ptr != NULL) {
                    GOOL_PUSH(obj, d_addr);
                    if (s_ptr != NULL) {
                        GOOL_PUSH(obj, s_addr);
                    }
                }
            } else {
                if (d_ptr != NULL) {
                    if (s_ptr != NULL) {
                        tmp_swap = *s_ptr;
                    }
                    GOOL_PUSH(obj, *d_ptr);
                    if (s_ptr != NULL) {
                        GOOL_PUSH(obj, tmp_swap);
                    }
                }
            }
            break;

        }
        case 0x14:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_val = TRANS_GOP_IN_ADDR(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_OUT(obj, ins & 0xFFF);
            if (d_ptr != NULL) {
                *d_ptr = s_val;
            }
            break;

        }
        case 0x27:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_OUT(obj, ins & 0xFFF);
            if (d_ptr != NULL) {
                *d_ptr = RAM_AT(GoolExecutable, obj->exec)->anims + (((u32)*s_ptr >> 8) << 2);
            }
            break;

        }
        case 0x19:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_val = *TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_OUT(obj, ins & 0xFFF);
            if (d_ptr != NULL) {
                *d_ptr = ABS(s_val);
            }
            break;

        }
        case 0x18:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            {
                u32 offset;
                d_ptr = TRANS_REG_OUT(obj, (ins >> 14) & 0x3F);
                offset = ins & 0x3FFF;
                if (offset != 0x3FFF) {
                    *d_ptr = RAM_AT(GoolExecutable, obj->exec)->code + offset * 4;
                } else {
                    *d_ptr = 0;
                }
            }
            break;

        }
        case 0x12:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_val = *TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_OUT(obj, ins & 0xFFF);
            if (d_ptr != NULL) {
                *d_ptr = (s_val == 0);
            }
            break;

        }
        case 0x17:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_val = *TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_OUT(obj, ins & 0xFFF);
            if (d_ptr != NULL) {
                *d_ptr = ~s_val;
            }
            break;

        }
        case 0x13:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            {
                s32 rate;
                s32 limit;
                s32 val;

                if (((ins >> 12) & 0xFFF) == 0xBF0) {
                    rate = GOOL_POP(obj);
                    limit = GOOL_POP(obj);
                } else {
                    rate = 0x100;
                    limit = *TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
                }
                s_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
                if (s_ptr != NULL) {
                    val = *s_ptr;
                    if (limit >= 0) {
                        val += rate;
                        if (val >= limit) {
                            val = (rate << 1) - limit;
                        }
                        limit = ABS(val);
                    } else {
                        val -= rate;
                        if (val <= limit) {
                            val = -limit - (rate << 1);
                        }
                        limit = -ABS(val);
                    }
                    d_ptr = TRANS_GOP_OUT(obj, ins & 0xFFF);
                    *d_ptr = val;
                    GOOL_PUSH(obj, limit);
                }
            }
            break;

        }
        case 0x22:
        case 0x25:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            {
                s32 rate;
                s32 target;

                if (((ins >> 12) & 0xFFF) == 0xBF0) {
                    rate = GOOL_POP(obj);
                    target = GOOL_POP(obj);
                } else {
                    rate = 0x100;
                    target = *TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
                }
                s_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
                if (s_ptr != NULL) {
                    s_val = *s_ptr;
                    if ((ins >> 24) == 0x22) {
                        if (rate < 0) {
                            rate = -rate;
                        }
                        GOOL_PUSH(obj, Gool_Seek(s_val, target, rate));
                    } else {
                        GOOL_PUSH(obj, Gool_ObjectRotate(s_val, target, rate, 0));
                    }
                } else {
                    GOOL_PUSH(obj, 0);
                }
            }
            break;

        }
        case 0x00:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, *s_ptr + *d_ptr);
            break;

        }
        case 0x01:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, *d_ptr - *s_ptr);
            break;

        }
        case 0x21:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_val = *TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, Gool_AngDiff(*d_ptr, s_val));
            break;

        }
        case 0x02:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, *s_ptr * *d_ptr);
            break;

        }
        case 0x03:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, *d_ptr / *s_ptr);
            break;

        }
        case 0x0D:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_val = *TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, *d_ptr % s_val);
            break;

        }
        case 0x10:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_val = *TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_val = *TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, Math_Randa(s_val - d_val) + d_val);
            break;

        }
        case 0x15:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_val = *TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_val = *TRANS_GOP_IN(obj, ins & 0xFFF);
            /* srav/sllv use the low five bits of the amount. */
            GOOL_PUSH(obj, s_val < 0 ? d_val >> (-s_val & 0x1F) : (s32)((u32)d_val << (s_val & 0x1F)));
            break;

        }
        case 0x07:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, *s_ptr & *d_ptr);
            break;

        }
        case 0x08:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, *s_ptr | *d_ptr);
            break;

        }
        case 0x0E:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, *s_ptr ^ *d_ptr);
            break;

        }
        case 0x0F:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_val = *TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, !((s_val & *d_ptr) ^ s_val));
            break;

        }
        case 0x04:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, (*s_ptr == *d_ptr));
            break;

        }
        case 0x09:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, (*s_ptr > *d_ptr));
            break;

        }
        case 0x0A:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, (*s_ptr >= *d_ptr));
            break;

        }
        case 0x0B:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, (*s_ptr < *d_ptr));
            break;

        }
        case 0x0C:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, (*s_ptr <= *d_ptr));
            break;

        }
        case 0x05:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, (*d_ptr && *s_ptr));
            break;

        }
        case 0x06:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, (*d_ptr || *s_ptr));
            break;

        }
        case 0x1A:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            GOOL_PUSH(obj, Gool_TestControls(ins & 0x1FFFFF, 0));
            break;

        }
        case 0x1B:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_val = *TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_val = *TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, d_val + SCALE_DT(s_val));
            break;

        }
        case 0x1D:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            {
                s32 period;
                s32 phase;
                s32 shift;
                s32 sin_val;

                period = *TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
                d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
                phase = (*d_ptr << 11) / period;
                shift = (period > 0xFFFF) << 1;
                sin_val = (Math_Sin(phase - 0x400) + 0x1000) >> shift;
                GOOL_PUSH(obj, (sin_val * period) >> (0xD - shift));
            }
            break;

        }
        case 0x1E:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_val = *TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_val = *TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, (s_val + g_draw_stamp[0]) % d_val);
            break;

        }
        case 0x1F:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            GOOL_PUSH(obj, g_cur_display_flags[*d_ptr >> 8]);
            break;

        }
        case 0x20:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            g_cur_display_flags[*d_ptr >> 8] = *s_ptr;
            break;

        }
        case 0x23:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            {
                GoolObj *link = OBJ_LINK(obj, (ins >> 12) & 7);
                GOOL_PUSH(obj, link->defaults[(ins >> 15) & 0x3F]);
            }
            break;

        }
        case 0x24:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            {
                GoolObj *link = OBJ_LINK(obj, (ins >> 12) & 7);
                u32 idx = (ins >> 15) & 0x3F;
                d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
                link->defaults[idx] = *(u16 *)d_ptr;
            }
            break;

        }
        case 0x1C:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            {
                s32 subarg;

                s_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
                subarg = (s32)(ins << 12) >> 27;
                switch ((ins >> 20) & 0xF) {
                case 0:
                    if (s_ptr != NULL) {
                        if (*s_ptr != 0) {
                            GOOL_PUSH(obj, RAM_AT(s32, *s_ptr)[subarg]);
                        } else {
                            goto push_zero;
                        }
                    }
                    break;

                case 1:
                case 6:
                    {
                        Vec3 v1;
                        Vec3 v2;
                        Vec3 *vec_a;
                        Vec3 *vec_b;
                        GoolObj *link;

                        if (((ins >> 20) & 0xF) == 6) {
                            vec_a = (Vec3 *)s_ptr;
                        } else {
                            vec_a = &obj->trans;
                        }
                        link = OBJ_LINK(obj, (ins >> 12) & 7);
                        vec_b = &link->trans;
                        if (subarg & 2) {
                            v1.x = vec_a->x;
                            v1.y = 0;
                            v1.z = vec_a->z;
                            v2.x = link->trans.x;
                            v2.y = 0;
                            v2.z = link->trans.z;
                            vec_a = &v1;
                            vec_b = &v2;
                        }
                        if (subarg & 1) {
                            GOOL_PUSH(obj, Math_EucDist(vec_a, vec_b));
                        } else {
                            GOOL_PUSH(obj, Math_ApxDist(vec_a, vec_b));
                        }
                    }
                    break;

                case 5:
                    {
                        GoolObj *link = OBJ_LINK(obj, (ins >> 12) & 7);
                        s32 diff = Math_AngDistXZ((s32 *)&obj->trans, (s32 *)&link->trans) - (obj->rot.y & 0xFFF);
                        if (ABS(diff) >= 0x800) {
                            diff = (-diff) & 0xFFF;
                        }
                        GOOL_PUSH(obj, diff);
                    }
                    break;

                case 2:
                    if (!(obj->status_b & 0x200200)) {
                        GOOL_PUSH(obj, Math_AngDistXZ((s32 *)&OBJ_LINK(obj, (ins >> 12) & 7)->trans, s_ptr));
                    } else {
                        GOOL_PUSH(obj, Math_AngDistXY((s32 *)&OBJ_LINK(obj, (ins >> 12) & 7)->trans, s_ptr));
                    }
                    break;

                case 3:
                    GOOL_PUSH(obj, ((s32 *)((u8 *)OBJ_LINK(obj, (ins >> 12) & 7) + 0x60))[(u32)*s_ptr >> 8]);
                    break;

                case 4:
                    {
                        GoolObj *link = OBJ_LINK(obj, (ins >> 12) & 7);
                        ((s32 *)((u8 *)link + 0x60))[(u32)*s_ptr >> 8] = GOOL_POP(obj);
                    }
                    break;

                case 7:
                    if (s_ptr != NULL) {
                        if (*s_ptr != 0 && (g_spawns[(u32)*s_ptr >> 8] & 1)) {
                            s32 i = 3;
                            s32 found = 0;
                            s32 pid = *s_ptr;
                            do {
                                ObjectRoot *root = &((ObjectRoot *)g_free_objects)[i++];
                                found = Gool_FindObject_inline((GoolObj *)root, Gool_VisitHasPidFlags, &pid);
                            } while (found == 0 && i < 5);
                            GOOL_PUSH(obj, found);
                        } else {
                            goto push_zero;
                        }
                    }
                    break;

                case 8:
                    if (s_ptr != NULL) {
                        if (subarg != 0) {
                            g_spawns[(u32)*s_ptr >> 8] &= ~2;
                        } else {
                            g_spawns[(u32)*s_ptr >> 8] |= 2;
                        }
                    }
                    break;

                case 10:
                    if (s_ptr != NULL) {
                        switch (subarg) {
                        case 0:
                            g_spawns[(u32)*s_ptr >> 8] &= ~4;
                            break;
                        case 1:
                            g_spawns[(u32)*s_ptr >> 8] |= 4;
                            break;
                        case 4:
                            {
                                u16 *p = g_level_spawns;
                                u16 tag = (*s_ptr >> 8) | (RAM_AT(LevelStateView, g_ns_page_pool[0])->level_id << 9);
                                while (*p != 0) {
                                    if (*p == tag) {
                                        if (p[1] != 0) {
                                            *p = 1;
                                        } else {
                                            *p = 0;
                                        }
                                        break;
                                    }
                                    p++;
                                }
                            }
                        case 2:
                            g_spawns[(u32)*s_ptr >> 8] &= ~8;
                            break;
                        case 5:
                            {
                                u16 *p = g_level_spawns;
                                u16 *free_slot = NULL;
                                s32 tag = (*s_ptr >> 8) | (RAM_AT(LevelStateView, g_ns_page_pool[0])->level_id << 9);
                                if (!(RAM_AT(ZoneHeaderView, RAM_AT(ResourceEntry, g_pads[0])->data)->flags2FC & 0x2000)) {
                                    while (*p != 0) {
                                        if (*p == (u16)tag) {
                                            goto set_bit_3;
                                        }
                                        if (*p == 1) {
                                            free_slot = p;
                                        }
                                        p++;
                                    }
                                    if (free_slot == NULL) {
                                        free_slot = p;
                                    }
                                    *free_slot = tag;
                                }
                            }
                        case 3:
                        set_bit_3:
                            g_spawns[(u32)*s_ptr >> 8] |= 8;
                            break;
                        case 9:
                            g_spawns[(u32)*s_ptr >> 8] |= 1;
                            break;
                        case 8:
                            g_spawns[(u32)*s_ptr >> 8] &= ~1;
                            break;
                        }
                    }
                    break;

                case 11:
                    switch (subarg) {
                    case 1:
                        GOOL_PUSH(obj, s_ptr ? (((g_spawns[(u32)*s_ptr >> 8] >> 1) ^ 1) & 1) : 0);
                        break;
                    case 2:
                        GOOL_PUSH(obj, s_ptr ? (g_spawns[(u32)*s_ptr >> 8] & 4) : 0);
                        break;
                    case 3:
                        GOOL_PUSH(obj, s_ptr ? (g_spawns[(u32)*s_ptr >> 8] & 8) : 0);
                        break;
                    }
                    break;

                case 9:
                    {
                        s32 zone;
                        if (s_ptr == NULL) {
                            zone = g_pads[0];
                        } else {
                            zone = Zone_FindNeighbor(RAM_AT(struct ZoneEntry, g_pads[0]), (struct ZqVec *)s_ptr);
                        }
                        if (zone < -0xFE) {
                            OBJ_LINK(obj, (ins >> 12) & 7)->zone = zone;
                        }
                    }
                    break;

                case 12:
                    switch (subarg) {
                    case 0:
                        Level_SaveState((struct SaveObj *)obj, (struct LevelSave *)g_checkpoint_save_state, 0);
                        break;
                    case 1:
                        Level_Restart((struct LevelSave *)g_checkpoint_save_state);
                        break;
                    case 2:
                        {
                            GoolObj *new_parent = (GoolObj *)&((ObjectRoot *)g_free_objects)[(u32)*s_ptr >> 8];
                            GoolObj *parent = RAM_AT(GoolObj, obj->parent);
                            GoolObj *prev;

                            if (parent != NULL) {
                                prev = RAM_AT(GoolObj, NODE_CHILDREN(parent));
                                if (prev == obj) {
                                    *NODE_CHILDREN_PTR(parent) = obj->next;
                                } else if (prev != NULL) {
                                    while (prev->next != Ram_Addr(obj)) {
                                        if (prev == NULL) {
                                            goto end_case_2;
                                        }
                                        prev = RAM_AT(GoolObj, prev->next);
                                        if (prev == NULL) {
                                            goto next_ins;
                                        }
                                    }
                                    prev->next = obj->next;
                                } else {
                                    goto next_ins;
                                }
                            }
                            obj->parent = Ram_Addr(new_parent);
                            obj->next = NODE_CHILDREN(new_parent);
                            *NODE_CHILDREN_PTR(new_parent) = Ram_Addr(obj);
                        end_case_2:;
                        }
                        break;
                    case 3:
                        Cd_Stub_22();
                        break;
                    case 4:
                        RAM_AT(GoolObj, *s_ptr)->zone = g_cur_zone_ptr[0];
                        break;
                    case 5:
                        Midi_ResetFadeStep();
                        break;
                    case 6:
                        Midi_TogglePlayback(*s_ptr);
                        break;
                    case 7:
                        Gool_ZoneObjectTerminateSingle();
                        break;
                    case 8:
                        {
                            GoolObj *link = OBJ_LINK(obj, (ins >> 12) & 7);
                            s32 dist = Math_EucDistXZ(&link->trans, (Vec3 *)s_ptr);
                            GOOL_PUSH(obj, ratan2(link->trans.y - s_ptr[1], dist));
                        }
                        break;
                    case 9:
                        gp_pbak_frame_idx = (u32)*s_ptr >> 8;
                        break;
                    case 10:
                        Ns_CdSeek((u32)*s_ptr >> 8);
                        break;
                    case 11:
                        Level_ResetGlobals(1);
                        break;
                    }
                    break;

                case 13:
                    {
                        GoolQuery query;
                        query.kind = (ins >> 15) & 0x1F;
                        query.obj = (&obj->self)[(ins >> 12) & 7];
                        query.result = 0;
                        query.best_dist = 0x7FFFFFFF;
                        query.max_dist = *s_ptr;
                        Gool_FindObject_inline(&g_gool_obj_list_4, Gool_VisitFindNearest, &query);
                        if (query.result != 0) {
                            GOOL_PUSH(obj, query.result);
                        } else {
                        push_zero:
                            GOOL_PUSH(obj, 0);
                        }
                    }
                    break;

                case 14:
                    {
                        GoolBound bound;
                        GoolObj *link = OBJ_LINK(obj, (ins >> 12) & 7);
                        bound.p1.x = link->trans.x + link->bound.p1.x;
                        bound.p1.y = link->trans.y + link->bound.p1.y;
                        bound.p1.z = link->trans.z + link->bound.p1.z;
                        bound.p2.x = link->trans.x + link->bound.p2.x;
                        bound.p2.y = link->trans.y + link->bound.p2.y;
                        bound.p2.z = link->trans.z + link->bound.p2.z;
                        GOOL_PUSH(obj, Level_TestPointInBound((struct ZqVec *)s_ptr, (struct ZqBound *)&bound));
                    }
                    break;

                case 15:
                    obj->misc_flag = Card_Control(subarg, *s_ptr);
                    break;
                }
            }
            break;

        }
        case 0x88:
        case 0x89:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            {
                u32 mode = (ins >> 22) & 3;
                u32 cond_type = (ins >> 20) & 3;
                s32 res;

                if (!(flags & 8)) {
                    return -0xE;
                }
                switch (cond_type) {
                case 0:
                    cond1 = 1;
                    break;
                case 1:
                    cond1 = *TRANS_REG_IN(obj, (ins >> 14) & 0x3F);
                    break;
                case 2:
                    cond1 = (*TRANS_REG_IN(obj, (ins >> 14) & 0x3F) == 0);
                    break;
                }
                if (cond1) {
                    flags |= 0x20;
                    out_state->flag = (opcode != 0x88);
                    switch (mode) {
                    case 2:
                        res = Gool_ObjectPopFrame(obj, NULL, 0);
                        if (res >= -0xFE) {
                            return res;
                        }
                        out_state->state = 0xFF;
                        return -0xFF;
                    case 1:
                        res = Gool_ObjectPopFrame(obj, NULL, 0);
                        if (res >= -0xFE) {
                            return res;
                        }
                        out_state->state = ins & 0x3FFF;
                        return -0xFF;
                    }
                } else if (mode == 0) {
                    obj->pc += ((s32)(ins << 22) >> 22) * 4;
                    obj->sp -= (ins >> 8) & 0x3C;
                }
            }
            break;

        }
        case 0x82:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            {
                u32 mode = (ins >> 22) & 3;
                u32 cond_type = (ins >> 20) & 3;
                s32 res;

                switch (cond_type) {
                case 0:
                    cond2 = 1;
                    break;
                case 1:
                    cond2 = *TRANS_REG_IN(obj, (ins >> 14) & 0x3F);
                    break;
                case 2:
                    cond2 = (*TRANS_REG_IN(obj, (ins >> 14) & 0x3F) == 0);
                    break;
                }
                if (cond2) {
                    switch (mode) {
                    case 2:
                        {
                            s32 saved_flags = flags;
                            res = Gool_ObjectPopFrame(obj, &saved_flags, flags);
                            if (flags & 8) {
                                if (flags & 0x20) {
                                    out_state->state = 0xFF;
                                    return -0xFF;
                                }
                                return -0x1A;
                            }
                            if ((flags & 1) || res >= -0xFE) {
                                if (flags & 0x10) {
                                    if (res < -0xFE) {
                                        GOOL_PUSH(obj, 0);
                                    }
                                }
                                return res;
                            }
                            if ((saved_flags & 0xFFFF) != 0xFFFF) {
                                flags = saved_flags;
                            }
                        }
                        break;
                    case 0:
                        obj->pc += ((s32)(ins << 22) >> 22) * 4;
                        obj->sp -= (ins >> 8) & 0x3C;
                        break;
                    case 1:
                        {
                            u32 status_c = obj->status_c;
                            u32 state_status_c = RAM_AT(GoolState, RAM_AT(GoolExecutable, obj->exec)->states)[ins & 0x3FFF].status_c;
                            s32 inv_state = obj->invincibility_state;
                            u32 mask;

                            if ((u32)(inv_state - 3) < 2 || inv_state == 2) {
                                mask = (status_c | 0x1002) & state_status_c;
                            } else {
                                mask = status_c & state_status_c;
                            }
                            if (mask == 0) {
                                res = GOOL_ChangeObjectState(obj, ins & 0x3FFF, 0, NULL);
                                if (res < -0xFE && (flags & 2)) {
                                    return res;
                                }
                                obj->sp -= 4;
                            }
                        }
                        break;
                    }
                }
            }
            break;

        }
        case 0x86:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            {
                u32 offset;
                GOOL_CreateObjectStackFrame_inline(obj, (ins >> 20) & 0xF, flags);
                offset = ins & 0x3FFF;
                if (offset != 0x3FFF) {
                    obj->pc = RAM_AT(GoolExecutable, obj->exec)->code + offset * 4;
                } else {
                    obj->pc = 0;
                }
                flags &= ~9;
            }
            break;

        }
        case 0x83:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            obj->anim_seq = RAM_AT(GoolExecutable, obj->exec)->anims + ((ins >> 5) & 0x7FC);
            obj->anim_frame = (ins & 0x7F) << 8;
            GOOL_PUSH(obj, (((ins >> 16) & 0x3F) << 24) | g_level_zone_flags[0]);
            switch ((ins >> 22) & 3) {
            case 0:
                if (obj->scale.x < 0) {
                    obj->scale.x = -obj->scale.x;
                }
                break;
            case 1:
                if (obj->scale.x > 0) {
                    obj->scale.x = -obj->scale.x;
                }
                break;
            case 2:
                obj->scale.x = -obj->scale.x;
                break;
            }
            if (obj->status_b & 0x18) {
                if (!Math_OutOfRange((struct ObjectFlagsView *)obj, &obj->trans, &RAM_AT(GoolObj, gp_crash_obj)->trans, 0x7D000, 0xAF000, 0x7D000) || (s32)obj->status_b < 0) {
                    Gool_ObjectLocalBound(obj, &obj->scale.x);
                }
            }
            if (flags & 4) {
                return -0xFF;
            }
            break;

        }
        case 0x84:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            obj->anim_frame = *d_ptr;
            GOOL_PUSH(obj, (((ins >> 16) & 0x3F) << 24) | g_level_zone_flags[0]);
            switch ((ins >> 22) & 3) {
            case 0:
                if (obj->scale.x < 0) {
                    obj->scale.x = -obj->scale.x;
                }
                break;
            case 1:
                if (obj->scale.x > 0) {
                    obj->scale.x = -obj->scale.x;
                }
                break;
            case 2:
                obj->scale.x = -obj->scale.x;
                break;
            }
            Gool_ObjectLocalBound(obj, &obj->scale.x);
            if (flags & 4) {
                return -0xFF;
            }
            break;

        }
        case 0x8B:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_val = *RAM_AT(s32, Gool_TranslateOutGop((struct GoolObjView *)obj, (ins >> 12) & 0xFFF, 0));
            s_ptr = RAM_AT(s32, Gool_TranslateOutGop((struct GoolObjView *)obj, ins & 0xFFF, 0));
            switch (s_val) {
            case 1:
                obj->misc_flag = Ns_Open((u32 *)s_ptr, 0, 1);
                break;
            case 6:
                obj->misc_flag = Ns_Open((u32 *)s_ptr, 1, 1);
                break;
            case 2:
                obj->misc_flag = Ns_Close((u32 *)s_ptr, 1);
                break;
            case 3:
                GOOL_PUSH(obj, Ns_Close((u32 *)s_ptr, 0));
                break;
            case 4:
                GOOL_PUSH(obj, Ns_CountAvailablePages());
                break;
            case 5:
                {
                    s32 n = *s_ptr;
                    d_val = Ns_CountAvailablePages2(RAM_AT(u32, obj->sp) - n, n);
                    obj->sp -= n * 4;
                    GOOL_PUSH(obj, d_val);
                }
                break;
            }
            break;

        }
        case 0x85:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            {
                Vec3 *vec_dst;

                vec_dst = &(&obj->trans)[(ins >> 12) & 7];
                s_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
                switch ((ins >> 18) & 7) {
                case 0:
                    if (s_ptr != NULL && obj->entity != 0) {
                        Vec3 v2;
                        v2 = *vec_dst;
                        Gool_ObjectOrientOnPath(obj, *s_ptr, (s32 *)&v2);
                        *vec_dst = v2;
                        obj->floor_y = vec_dst->y;
                    }
                    break;

                case 2:
                    vec_dst->x = ((Math_Sin(obj->ang_vel.y & 0xFFF) >> 4) * *s_ptr) >> 8;
                    if (!(obj->status_b & 0x200200)) {
                        vec_dst->z = ((Math_Cos(obj->ang_vel.y & 0xFFF) >> 4) * *s_ptr) >> 8;
                    } else {
                        vec_dst->y = ((Math_Cos(obj->ang_vel.y & 0xFFF) >> 4) * *s_ptr) >> 8;
                    }
                    obj->speed = *s_ptr;
                    break;

                case 4:
                case 5:
                    {
                        Vec3 v1;
                        Vec3 *vec_src;
                        Vec3 *rot_ptr;
                        Vec3 *scale_ptr = NULL;

                        v1.z = *s_ptr;
                        v1.y = GOOL_POP(obj);
                        v1.x = GOOL_POP(obj);
                        vec_src = &(&obj->trans)[(ins >> 15) & 7];
                        if (((ins >> 18) & 7) == 4) {
                            rot_ptr = &obj->rot;
                            scale_ptr = &obj->scale;
                        } else {
                            rot_ptr = &obj->ang_vel;
                        }
                        Gool_Transform(&v1, vec_dst, rot_ptr, scale_ptr, vec_src);
                    }
                    break;

                case 6:
                    {
                        Vec3 v3;
                        Vec3 v4;
                        GoolObj *link = OBJ_LINK(obj, (ins >> 21) & 7);
                        GoolAnimSeq *anim_seq = RAM_AT(GoolAnimSeq, link->anim_seq);
                        if (anim_seq != NULL && anim_seq->type == 1) {
                            GoolAnimFrame *frame = RAM_AT(GoolAnimFrame, RAM_AT(GoolAnimEntry, Ns_Lookup(&anim_seq->eid))->items[(u32)link->anim_frame >> 8]);
                            GoolAnimEntry *model_group = RAM_AT(GoolAnimEntry, RAM_AT(GoolAnimEntry, Ns_Lookup(&RAM_AT(GoolAnimSeq, link->anim_seq)->eid))->items[(u32)link->anim_frame >> 8]);
                            GoolAnimModel *model;
                            GoolAnimVertex *vert;

                            model_group = RAM_AT(GoolAnimEntry, Ns_Lookup((u32 *)model_group + 1));
                            model = RAM_AT(GoolAnimModel, model_group->items[0]);
                            vert = &frame->verts[(u32)*s_ptr >> 8];
                            v3.x = ((frame->offset.x - 0x80) + vert->x) << 10;
                            v3.y = ((frame->offset.y - 0x80) + vert->y) << 10;
                            v3.z = ((frame->offset.z - 0x80) + vert->z) << 10;
                            v4.x = (link->scale.x * model->scale.x) >> 12;
                            v4.y = (link->scale.y * model->scale.y) >> 12;
                            v4.z = (link->scale.z * model->scale.z) >> 12;
                            Gool_Transform(&v3, &link->trans, &link->rot, &v4, vec_dst);
                        }
                    }
                    break;

                case 1:
                    {
                        Vec3 *vec_src = &(&obj->trans)[(ins >> 15) & 7];
                        Gool_Project(vec_dst, vec_src);
                        if (s_ptr[0] != 0) {
                            s32 z_scaled = vec_src->z >> 8;
                            if (z_scaled != 0) {
                                s_ptr[0] = (s_ptr[0] * 280) / z_scaled;
                                s_ptr[1] = (s_ptr[1] * 280) / z_scaled;
                                s_ptr[2] = (s_ptr[2] * 280) / z_scaled;
                            }
                        }
                    }
                    break;

                case 7:
                    Gool_Transform2(vec_dst, &(&obj->trans)[(ins >> 15) & 7], 0);
                    break;
                }
            }
            break;

        }
        case 0x8E:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            {
                Vec3 *vec_src;

                vec_src = &(&obj->trans)[(ins >> 12) & 7];
                Gool_TranslateOutGop((struct GoolObjView *)obj, ins & 0xFFF, 0);
                switch ((ins >> 18) & 7) {
                case 0:
                    {
                        Vec3 va;
                        Vec3 vb;

                        va = obj->trans;
                        vb = *vec_src;
                        va.y += obj->bound.p2.y / 2;
                        obj->misc_flag = Zone_ReboundVector((struct ZqVec *)&va, (struct ZqVec *)&vb);
                        if ((ins & 0xFFF) != 0xBE0) {
                            *(Vec3 *)TRANS_GOP_OUT(obj, ins & 0xFFF) = vb;
                        }
                        (&obj->trans)[(ins >> 15) & 7] = va;
                    }
                    break;

                case 1:
                    {
                        Vec3 v_tmp;
                        Vec3 va;
                        /* Never set: the original stores a stale stack slot. */
                        Vec3 vb = {0, 0, 0};

                        v_tmp = obj->trans;
                        va = obj->trans;
                        if (Ram_Addr(obj) == gp_crash_obj || obj->parent == gp_crash_obj) {
                            Zone_FindNearestObjectNode2((struct SubObj *)obj, (struct ZqVec *)&v_tmp);
                        }
                        obj->misc_flag = Zone_FindNearestObjectNode((struct SubObj *)obj, (struct ZqVec *)&va);
                        if ((ins & 0xFFF) != 0xBE0) {
                            *(Vec3 *)TRANS_GOP_OUT(obj, ins & 0xFFF) = vb;
                        }
                        (&obj->trans)[(ins >> 15) & 7] = va;
                    }
                    break;

                case 2:
                case 3:
                case 4:
                case 5:
                    {
                        Vec3 va;
                        /* Never set: the original stores a stale stack slot. */
                        Vec3 vb = {0, 0, 0};
                        u32 subop = (ins >> 18) & 7;
                        s32 arg3 = (u32)(subop - 4) < 2;
                        s32 is_odd = (subop == 5 || subop == 3);
                        ResourceEntry *zone;
                        s32 mode_flags;
                        Vec3 *va_ptr;

                        va = obj->trans;
                        va_ptr = &va;
                        zone = RAM_AT(ResourceEntry, obj->zone ? obj->zone : g_pads[0]);
                        if (RAM_AT(ZoneHeaderView, zone->data)->flags2FC & 1) {
                            mode_flags = is_odd ? 6 : 2;
                        } else {
                            mode_flags = is_odd ? 5 : 1;
                        }
                        Zone_FindNearestObjectNode3((struct SubObj *)obj, (struct ZqVec *)va_ptr, mode_flags, !arg3);
                        if ((ins & 0xFFF) != 0xBE0) {
                            *(Vec3 *)TRANS_GOP_OUT(obj, ins & 0xFFF) = vb;
                        }
                        (&obj->trans)[(ins >> 15) & 7] = va;
                    }
                    break;

                case 6:
                    Zone_ColorsScaleSeekByEntityNode((struct SubObj *)obj);
                    break;
                }
            }
            break;

        }
        case 0x87:
        case 0x90:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            recipient = OBJ_LINK(obj, (ins >> 21) & 7);
        case 0x8F:
            {
                /* The handler receives the arguments by address. */
                u32 args_addr = Ram_StackAlloc(16 * sizeof(s32));
                s32 *args = RAM_AT(s32, args_addr);
                s32 *event_ptr;
                s32 cond;
                s32 count;
                s32 i;

                event_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
                obj->status_a &= ~0x20000;
                cond = *TRANS_REG_IN(obj, (ins >> 12) & 0x3F);
                if (cond != 0 && (opcode == 0x8F || recipient != NULL) && event_ptr != NULL) {
                    count = (ins >> 18) & 7;
                    for (i = 0; i < count; i++) {
                        args[i] = RAM_AT(s32, obj->sp)[i - count];
                    }
                    switch (opcode) {
                    case 0x87:
                        Gool_ExecStateCode((struct GoolObjView *)obj, (struct GoolObjView *)recipient, *event_ptr, count, args);
                        break;
                    case 0x90:
                        Gool_SendToColliders2((struct Node *)obj, (struct Node *)recipient, *event_ptr, (ins >> 21) & 7, count, args_addr);
                        break;
                    case 0x8F:
                        Gool_SendToColliders((struct Node *)obj, *event_ptr, (ins >> 21) & 7, count, args);
                        break;
                    }
                } else {
                    obj->misc_flag = 0;
                }
                Ram_StackFree(16 * sizeof(s32));
                if (!(obj->status_a & 0x20000)) {
                    obj->sp -= (ins >> 16) & 0x1C;
                } else {
                    if (flags & 2) {
                        return -0xFF;
                    }
                    obj->sp -= 4;
                }
            }
            break;

        }
        case 0x8A:
        case 0x91:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            {
                s32 count = ins & 0x3F;
                s32 arg_count = (ins >> 20) & 0xF;
                s32 i;

                if (count == 0) {
                    d_val = arg_count + 1;
                    count = RAM_AT(s32, obj->sp)[-d_val];
                    arg_count = d_val;
                }
                for (i = 0; i < count; i++) {
                    s32 child = GOOL_CreateObject(
                        obj,
                        (ins >> 12) & 0xFF,
                        (ins >> 6) & 0x3F,
                        (ins >> 20) & 0xF,
                        RAM_AT(s32, obj->sp) - ((ins >> 20) & 0xF),
                        opcode == 0x91);
                    if (child < -0xFE) {
                        RAM_AT(GoolObj, child)->creator = Ram_Addr(obj);
                        obj->misc_flag = child;
                    } else {
                        obj->misc_flag = 0;
                    }
                }
                obj->sp -= arg_count * 4;
            }
            break;

        }
        case 0x8C:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            s_ptr = TRANS_GOP_IN(obj, (ins >> 12) & 0xFFF);
            d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
            obj->voice_id = Audio_VoiceCreate((struct GoolObjectAudio *)obj, (u32 *)d_ptr, *(s16 *)s_ptr);
            break;

        }
        case 0x8D:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            {
                u32 mode;
                u32 reg;
                s32 val;

                d_ptr = TRANS_GOP_IN(obj, ins & 0xFFF);
                mode = (ins >> 20) & 0xF;
                reg = (ins >> 12) & 0x3F;
                if (reg == 0) {
                    val = 0;
                } else {
                    val = *TRANS_REG_IN(obj, reg);
                }
                if (mode == 0xF) {
                    mode = 0x80000000;
                }
                if (ins & 0x40000) {
                    mode |= 0x40000000;
                }
                if (ins & 0x80000) {
                    mode |= 0x20000000;
                }
                Audio_Control(val, mode, d_ptr, (struct GoolObjectAudio *)obj);
            }
            break;

        }
        default:
        {
            s32 *s_ptr; s32 *d_ptr; s32 s_val; s32 d_val;
            return -0xE;
        }
        }
    }
}
