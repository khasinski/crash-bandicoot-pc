#include "common.h"

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
    RAM_REF(GoolCodeHeader) header;
    RAM_REF(u8) code;
    RAM_REF(u8) data;
    RAM_REF(u16) states;
    RAM_REF(void) word_20;
    RAM_REF(u32) anims;
} GoolExecutable;

typedef struct GoolDefaults {
    u8 bytes[0x2A];
    u16 intensity_r, intensity_g, intensity_b;
} GoolDefaults;

typedef struct ResourceEntry {
    u8 reserved_00[0x10];
    RAM_REF(u8) data;
} ResourceEntry;

typedef struct GoolObj {
    s32 kind;
    s32 word_04;
    u8 reserved_08[0x18];
    RAM_REF(GoolExecutable) exec;
    s32 word_24;
    RAM_REF(ResourceEntry) zone;
    s32 word_2c;
    GoolDefaults defaults;
    RAM_REF(struct GoolObj) self;
    RAM_REF(struct GoolObj) parent;
    RAM_REF(struct GoolObj) next;
    RAM_REF(struct GoolObj) children;
    s32 creator;
    RAM_REF(void) player;
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
    RAM_REF(u32) sp;
    RAM_REF(u32) pc;
    RAM_REF(u32) fp;
    RAM_REF(u32) tp;
    RAM_REF(u32) ep;
    s32 result;
    s32 misc_flag;
    s32 arg0;
    s32 arg1;
    s32 arg2;
    s32 arg3;
    RAM_REF(void) anim_seq;
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

typedef struct NsdTable {
    u8 reserved_00[0x14];
    s32 items[1];
} NsdTable;

#define CRASH_EXT_GOOL_UPDATE_OBJECT_0
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_UPDATE_OBJECT_0

typedef struct ZoneEntity {
    u8 reserved_00[4];
    u16 flags;
    u8 group;
    u8 reserved_07;
    u16 id;
    u16 path_length;
    s16 rot_x;
    s16 rot_y;
    s16 rot_z;
    u8 type;
    u8 subtype;
    s16 x;
    s16 y;
    s16 z;
} ZoneEntity;

typedef struct ObjectRoot {
    s32 kind;
    RAM_REF(GoolObj) word04;
} ObjectRoot;

typedef struct ZoneHeaderView {
    u8 reserved_00[0x204];
    s32 path_index;
    s32 path_count;
    u8 reserved_20c[0xD0];
    u32 flags2DC;
} ZoneHeaderView;

typedef struct ZoneEntryView {
    u32 word00;
    u32 id;
    s32 kind;
    s32 word0C;
    RAM_REF(void) items[1];
} ZoneEntryView;

#define CRASH_EXT_GOOL_UPDATE_OBJECT_1
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_UPDATE_OBJECT_1

s32 GOOL_UpdateObject(GoolObj *obj, s32 flag) {
    u32 update_flag;
    GoolCodeHeader *header;
    s32 res;
    u32 sp;
    u32 wait_entry;

    if (obj->word_04 != 3) {
        return -0xFF;
    }
    if (Ram_Addr(obj) == gp_crash_obj) {
        Pad_Update();
    }
    g_savestate[0] = Ram_Addr(obj);
    update_flag = g_cur_lid_ro[0] & 8;
    if (update_flag != 0) {
        if (((obj->status_b & 0x2000000) || (obj->state_flags & 0x20000)) && (g_cur_lid_ro[0] & 0x8000)) {
            update_flag = 1;
        } else {
            header = RAM_AT(GoolCodeHeader, RAM_AT(GoolExecutable, obj->exec)->header);
            switch (header->word_04) {
            case 0x100:
                update_flag = g_cur_lid_ro[0] & 0x20;
                break;
            case 0x300:
            case 0x500:
            case 0x600:
                update_flag = g_cur_lid_ro[0] & 0x80;
                break;
            case 0x200:
                update_flag = g_cur_lid_ro[0] & 0x100;
                if (header->word_00 == 4 && (obj->subtype == 4 || obj->subtype == 7)) {
                    flag = 1;
                }
                break;
            case 0x400:
                update_flag = g_cur_lid_ro[0] & 0x400;
                break;
            }
        }
    }
    if (flag != 0 && update_flag != 0) {
        if ((obj->status_b & 0x10000000) && obj->arg3 != 0) {
            obj->arg3--;
            if (obj->arg3 == 0) {
                obj->status_b &= ~0x10000000;
            }
        } else {
            obj->arg1 = g_level_zone_flags[0];
            if ((obj->status_b & 0x10) && RAM_AT(GoolObj, gp_crash_obj)->arg1 == obj->arg1) {
                Gool_ObjectControlDir(obj);
            }
            if (obj->tp != 0) {
                GOOL_CreateObjectStackFrame((struct GoolRuntimeObject *)obj, 0, 0xFFFF);
                obj->pc = obj->tp;
                res = GOOL_InterpretObject(obj, 3, 0);
                if (res >= -0xFE) {
                    return res;
                }
            }
            sp = obj->sp;
            wait_entry = RAM_AT(u32, sp)[-1];
            if (g_level_zone_flags[0] - (wait_entry & 0xFFFFFF) >= (wait_entry >> 24)) {
                obj->sp = sp - 4;
                res = GOOL_InterpretObject(obj, 4, 0);
                if (res >= -0xFE) {
                    return res;
                }
            }
            Gool_ObjectColors(obj);
            Gool_ObjectPhysics(obj);
            obj->status_a &= ~0x20;
        }
    }
    if (obj->anim_seq != 0 && !(obj->status_b & 0x100) && (g_cur_lid_ro[0] & 4)) {
        if (((obj->status_b & 0x2000000) || (obj->state_flags & 0x20000)) && (g_cur_lid_ro[0] & 0x4000)) {
            update_flag = 1;
        } else {
            update_flag = 0;
            switch (RAM_AT(GoolCodeHeader, RAM_AT(GoolExecutable, obj->exec)->header)->word_04) {
            case 0x100:
                update_flag = g_cur_lid_ro[0] & 0x10;
                break;
            case 0x300:
            case 0x500:
            case 0x600:
                update_flag = g_cur_lid_ro[0] & 0x40;
                break;
            case 0x200:
                update_flag = g_cur_lid_ro[0] & 0x200;
                break;
            case 0x400:
                update_flag = g_cur_lid_ro[0] & 0x800;
                break;
            }
        }
        if (update_flag != 0) {
            Gool_ObjectTransform(obj);
        }
    }
    g_savestate[0] = 0;
    return -0xFF;
}

typedef struct GoolFrag {
    u8 tex[8];
    s16 x1;
    s16 y1;
    s16 x2;
    s16 y2;
} GoolFrag;

typedef struct GoolSpriteAnim {
    u8 type;
    u8 reserved_01[3];
    u32 eid;
    u8 frames[1][8];
} GoolSpriteAnim;

typedef struct GoolAnimSeq {
    u8 type;
    u8 reserved_01[3];
    u32 eid;
    s32 count;
    GoolFrag frags[1];
} GoolAnimSeq;

typedef struct GoolBound {
    s32 x1;
    s32 y1;
    s32 z1;
    s32 x2;
    s32 y2;
    s32 z2;
} GoolBound;

typedef struct GoolItemsEntry {
    u32 magic;
    u32 eid;
    s32 kind;
    s32 item_count;
    RAM_REF(u8) items[1];
} GoolItemsEntry;

typedef struct RenderBufferView {
    u8 reserved_00[0x88];
    u32 ordering_table[0x800];
} RenderBufferView;

typedef struct CamTransformState {
    u8 mat7E4[0x60];
    u8 mat844[0x20];
    u8 vec864[0x24];
    u8 vec888[0x48];
} CamTransformState;

typedef struct RenderProjectionState {
    s32 distance;
    s16 matrix[9];
} RenderProjectionState;

typedef struct ZoneDrawHeader {
    u8 reserved_00[0x2E8];
    s32 depth_offset;
    u8 reserved_2ec[0x2C];
    GoolDefaults obj_defaults;
    GoolDefaults player_defaults;
} ZoneDrawHeader;

#define CRASH_EXT_GOOL_UPDATE_OBJECT_2
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_UPDATE_OBJECT_2

#define ABS(x) ((x) < 0 ? -(x) : (x))

void Gool_ObjectTransform(GoolObj *obj) {
    GoolAnimSeq *anim_seq;
    u32 anim_frame;
    GoolBound bound;
    ResourceEntry *zone;

    anim_seq = RAM_AT(GoolAnimSeq, obj->anim_seq);
    anim_frame = (u32)obj->anim_frame >> 8;
    switch (anim_seq->type) {
    case 2:
        {
            GoolSpriteAnim *anim2 = (GoolSpriteAnim *)anim_seq;
            s32 shift = ABS(obj->vectors.scale[0]) / 27279;
            if (Gte_RCalcSpriteRotMatrix(
                    &obj->vectors,
                    (obj->status_b & 0x200000) ? (u8 *)(*(CamTransformState *)g_m_view).vec864 : (u8 *)(*(CamTransformState *)g_m_view).vec888,
                    obj->status_b & 0x200,
                    shift,
                    (obj->status_b & 0x200000) ? (u8 *)(*(CamTransformState *)g_m_view).mat7E4 : (u8 *)(*(CamTransformState *)g_m_view).mat844,
                    g_view_proj.distance,
                    RAM_AT(ZoneDrawHeader, RAM_AT(ResourceEntry, g_pads[0])->data)->depth_offset >> 8,
                    ((obj->status_b >> 18) ^ 1) & 1)) {
                u32 tex = Ns_Lookup(&anim2->eid);
                Gte_RTransformSprite(
                    RAM_AT(RenderBufferView, g_ot_cur[0])->ordering_table,
                    anim2->frames[anim_frame],
                    tex,
                    200 << shift,
                    obj->cam_zoom + (0x800 - g_view_proj.distance / 2),
                    Gpu_GetCurPrimsTail(),
                    g_wgeo_tex_anim_masks);
            }
        }
        break;
    case 5:
        {
            s32 shift = ABS(obj->vectors.scale[0]) / 27279;
            if (Gte_RCalcSpriteRotMatrix(
                    &obj->vectors,
                    &(*(CamTransformState *)g_m_view).vec888,
                    obj->status_b & 0x200,
                    shift,
                    &(*(CamTransformState *)g_m_view).mat844,
                    g_view_proj.distance,
                    RAM_AT(ZoneDrawHeader, RAM_AT(ResourceEntry, g_pads[0])->data)->depth_offset >> 8,
                    ((obj->status_b >> 18) ^ 1) & 1)) {
                GoolAnimSeq *anim5 = anim_seq;
                s32 i = 0;
                GoolFrag *frag = &anim5->frags[anim_frame * anim5->count];
                u32 tex = Ns_Lookup(&anim5->eid);
                for (; i < anim5->count; frag++, i++) {
                    bound.x1 = frag->x1 << shift;
                    bound.y1 = frag->y1 << shift;
                    bound.x2 = frag->x2 << shift;
                    bound.y2 = frag->y2 << shift;
                    Gfx_TransformFragment((u32 *)frag, obj->cam_zoom, tex, (struct Rect9398 *)&bound, RAM_AT(RenderBufferView, g_ot_cur[0])->ordering_table);
                }
            }
        }
        break;
    case 4:
        Gool_TextObjectTransform(obj, anim_seq, anim_frame, RAM_AT(RenderBufferView, g_ot_cur[0])->ordering_table, obj->status_b & 0x400);
        break;
    case 1:
        if (RAM_AT(GoolItemsEntry, Ns_Lookup(&anim_seq->eid))->kind == 20) {
            Gfx_TransformCvtx(
                RAM_AT(u8, RAM_AT(GoolItemsEntry, Ns_Lookup(&RAM_AT(GoolAnimSeq, obj->anim_seq)->eid))->items[(u32)obj->anim_frame >> 8]),
                RAM_AT(RenderBufferView, g_ot_cur[0])->ordering_table,
                obj);
        } else {
            Gfx_LoadWorlds(
                RAM_AT(u8, RAM_AT(GoolItemsEntry, Ns_Lookup(&RAM_AT(GoolAnimSeq, obj->anim_seq)->eid))->items[(u32)obj->anim_frame >> 8]),
                RAM_AT(RenderBufferView, g_ot_cur[0])->ordering_table,
                obj);
        }
        if (obj->status_b & 0x100000) {
            if (Ram_Addr(obj) == gp_crash_obj) {
                obj->defaults = RAM_AT(ZoneDrawHeader, RAM_AT(ResourceEntry, obj->zone)->data)->player_defaults;
            } else {
                zone = RAM_AT(ResourceEntry, obj->zone ? obj->zone : g_pads[0]);
                obj->defaults = RAM_AT(ZoneDrawHeader, zone->data)->obj_defaults;
            }
        }
        break;
    }
}

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

s32 Gool_TextStringTransform(GoolObj *obj, u8 *str, s32 start_x, GoolFont *font, u32 tex, s32 shift, u32 *ot) {
    GoolBound glyph_bound;
    GoolBound bg_bound;
    u8 num_buf[8];
    s32 max_x;
    s32 has_bg;
    s32 color_flag;
    s32 cur_y;
    s32 scale_x;
    s32 scale_y;
    s32 len;
    u8 axis;
    u8 ch;
    GoolGlyph *glyph;
    s32 x2;
    s32 y2;
    s32 y1;

    color_flag = 1;
    cur_y = 0;
    max_x = 0;
    has_bg = 0;
    if (ot != NULL) {
        has_bg = obj->status_b & 0x4000000;
        if (has_bg) {
            bg_bound.x1 = 999999;
            bg_bound.y1 = 999999;
            bg_bound.x2 = -999999;
            bg_bound.y2 = -999999;
        }
    }
    glyph_bound.x1 = start_x;
    while (*str != '\0') {
        scale_x = 400;
        scale_y = 400;
        if (*str == '~') {
        tilde_loop:
            str++;
            switch (*str++) {
            case 'c':
                color_flag = (*str++ == '1');
                if (*str == '~') {
                    goto tilde_loop;
                }
                break;
            case 'p':
                if (*RAM_AT(u32, obj->sp - ((*str++ - 0x2E) << 2)) == 1) {
                    str++;
                }
                if (*str == '\0') {
                    goto done;
                }
                if (*str == '~') {
                    goto tilde_loop;
                }
                break;
            case 's':
                len = 0;
                axis = *str++;
                while ((ch = *str++) != '~') {
                    num_buf[len++] = ch;
                }
                num_buf[len] = '\0';
                if (axis == 'x') {
                    scale_x = BIOS_AToI((char *)num_buf);
                } else {
                    scale_y = BIOS_AToI((char *)num_buf);
                }
                if (*str == '~') {
                    goto tilde_loop;
                }
                break;
            case 'n':
            case '%':
                cur_y -= font->glyphs[0].height << shift;
                glyph_bound.x1 = start_x;
                goto next_char;
            }
        }
        x2 = (scale_x << shift) + glyph_bound.x1;
        glyph = &font->glyphs[*str - ' '];
        glyph_bound.x2 = x2;
        glyph_bound.y2 = cur_y + (glyph->height << shift);
        glyph_bound.y1 = glyph_bound.y2 - (scale_y << shift);
        if (glyph_bound.x2 > max_x) {
            max_x = glyph_bound.x2;
        }
        if (ot != NULL && glyph->word00 != 0) {
            if (has_bg) {
                bg_bound.x1 = bg_bound.x1 < glyph_bound.x1 ? bg_bound.x1 : glyph_bound.x1;
                bg_bound.y1 = bg_bound.y1 < glyph_bound.y1 ? bg_bound.y1 : glyph_bound.y1;
                bg_bound.x2 = bg_bound.x2 < glyph_bound.x2 ? glyph_bound.x2 : bg_bound.x2;
                bg_bound.y2 = bg_bound.y2 < glyph_bound.y2 ? glyph_bound.y2 : bg_bound.y2;
            }
            Gfx_TransformFontChar((struct LightCols9398 *)obj, (u32 *)glyph, obj->cam_zoom, tex, (struct Rect9398 *)&glyph_bound, ot, color_flag);
        }
        glyph_bound.x1 += glyph->width << shift;
    next_char:
        str++;
    }
done:
    if (has_bg && ot != NULL) {
        GoolGlyph *backdrop = &font->backdrop;
        GoolBound *bg_ptr = &bg_bound;
        if (backdrop->word00 != 0) {
            bg_bound.x1 -= 100;
            bg_bound.y1 -= 0;
            bg_bound.x2 += 100;
            bg_bound.y2 += 0;
            Gfx_TransformFragment((u32 *)backdrop, obj->cam_zoom - 10, tex, (struct Rect9398 *)bg_ptr, ot);
        }
    }
    return max_x;
}
