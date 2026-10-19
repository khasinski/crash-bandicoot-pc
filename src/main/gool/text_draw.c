#include "common.h"
#include "render.h"
#include "ns.h"

typedef struct GoolVectors {
    s32 trans[3];
    s32 rot[3];
    s32 scale[3];
} GoolVectors;

typedef struct GoolCodeHeader {
    s32 word_00;
    s32 word_04;
    s32 word_08;
    s32 word_0c;
    s32 word_10;
} GoolCodeHeader;

typedef struct GoolExecutable {
    u8 reserved_00[0x10];
    GoolCodeHeader *header;
    u8 *code;
    u8 *data;
    u16 *states;
    void *word_20;
    u32 *anims;
} GoolExecutable;

typedef struct GoolDefaults {
    u8 bytes[0x2A];
    u16 intensity_r, intensity_g, intensity_b;
} GoolDefaults;

typedef struct GoolObj {
    s32 kind;
    s32 word_04;
    u8 reserved_08[0x18];
    GoolExecutable *exec;
    s32 word_24;
    ResourceEntry *zone;
    s32 word_2c;
    GoolDefaults defaults;
    struct GoolObj *self;
    struct GoolObj *parent;
    struct GoolObj *next;
    struct GoolObj *children;
    s32 creator;
    void *player;
    s32 collider;
    s32 interrupter;
    GoolVectors vectors;
    s32 vel_x;
    s32 vel_y;
    s32 vel_z;
    s32 target_rot_x;
    s32 target_rot_y;
    s32 target_rot_z;
    s32 rot_vel_x;
    s32 rot_vel_y;
    s32 rot_vel_z;
    u32 status_a;
    u32 status_b;
    u32 status_c;
    s32 subtype;
    s32 pid_flags;
    u32 *sp;
    u32 *pc;
    u32 *fp;
    u32 *tp;
    u32 *ep;
    s32 result;
    s32 misc_flag;
    s32 arg0;
    s32 arg1;
    s32 arg2;
    s32 arg3;
    void *anim_seq;
    s32 anim_frame;
    void *entity;
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

typedef struct GoolAnimSeq {
    u8 type;
    u8 reserved_01[3];
    u32 eid;
    s32 count;
    u8 data[1];
} GoolAnimSeq;

typedef struct CamTransformState {
    u8 mat7E4[0x60];
    u8 mat844[0x20];
    u8 vec864[0x24];
    u8 vec888[0x48];
} CamTransformState;

typedef struct GoolGlyph {
    u32 word00;
    u32 word04;
    s16 width;
    s16 height;
} GoolGlyph;

typedef struct GoolFont {
    u32 word_00;
    u32 eid;
    GoolGlyph glyphs[63];
    GoolGlyph backdrop;
} GoolFont;

#define ABS(x) ((x) < 0 ? -(x) : (x))

void Gool_TextObjectTransform(GoolObj *obj, GoolAnimSeq *anim_seq, u32 anim_frame, u32 *ot, u32 center_flag)
{
    u8 buf[80];
    s32 shift;
    GoolAnimSeq *anim4;
    u8 *str;
    s32 i;
    u32 font_offset;
    GoolFont *font;
    void *tex;
    u8 *p;
    s32 width;

    shift = ABS(obj->vectors.scale[0]) / 27279;
    if (Gte_RCalcSpriteRotMatrix(
            &obj->vectors,
            &(*(CamTransformState *)g_m_view).vec888,
            obj->status_b & 0x200,
            shift,
            &(*(CamTransformState *)g_m_view).mat844,
            g_view_proj.distance,
            0x3C00,
            ((obj->status_b >> 18) ^ 1) & 1)) {
        anim4 = anim_seq;
        str = anim4->data;
        i = 0;
        while (i < (s32)anim_frame) {
            if (*str++ == '\0') {
                i++;
            }
        }
        font_offset = (u32)obj->invincibility_state >> 8;
        font = (GoolFont *)(obj->exec->anims + (font_offset ? font_offset : anim4->count));
        tex = Ns_Lookup(&font->eid);
        sprintf(buf, str, *(obj->sp - 2), *(obj->sp - 3), *(obj->sp - 4), *(obj->sp - 5));
        str = buf;
        p = str;
        while (*p != '\0') {
            p++;
        }
        if (p != str) {
            p--;
            while (p != str) {
                if (*p != ' ') {
                    break;
                }
                p--;
            }
            p[1] = '\0';
        }
        width = 0;
        if (center_flag) {
            width = Gool_TextStringTransform(obj, str, 0, font, tex, shift, NULL);
        }
        Gool_TextStringTransform(obj, str, -width / 2, font, tex, shift, ot);
    }
}
