#include "common.h"
#include "object_links.h"
#include "gool.h"
#include "zone.h"
#include "ns.h"

s32 Gool_SendToColliders2(Node *arg0, Node *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    struct { RAM_REF(Node) sender; s32 word04, word08, word0C, word10, word14; } message;
    void *context = &message;
    Node *node, *next, *child, *next_child;

    message.sender = Ram_Addr(arg0);
    message.word04 = arg2;
    message.word0C = arg3;
    message.word10 = 0;
    message.word08 = arg4;
    message.word14 = arg5;
    if (arg1->kind == 2) {
        node = RAM_AT(Node, arg1->word04);
    } else {
        node = RAM_AT(Node, arg1->children);
    }
    if (node != NULL) {
        do {
            child = RAM_AT(Node, node->children);
            next = RAM_AT(Node, node->next);
            if (child != NULL) {
                do {
                    next_child = RAM_AT(Node, child->next);
                    if (GOOL_TraverseObjectTreePostorder(child, Gool_SendIfColliding, context) == -25) {
                        Gool_ObjectKill((struct GoolObj *)node, 0);
                    }
                    child = next_child;
                } while (child != NULL);
            }
            if (Gool_SendIfColliding(node, context) == -25) {
                Gool_ObjectKill((struct GoolObj *)arg1, 0);
            }
            node = next;
        } while (node != NULL);
    }
    return -0xFF;
}

typedef struct GoolScriptHeader {
    u8 reserved_00[0x14];
    RAM_REF(u8) code;
} GoolScriptHeader;

typedef struct GoolStackObj {
    u8 reserved_00[0x20];
    RAM_REF(GoolScriptHeader) script;
    u8 reserved_24[0xDC - 0x24];
    RAM_REF(u32) sp;
    RAM_REF(u8) pc;
    RAM_REF(u32) fp;
} GoolStackObj;

/* sp, fp and pc are PS1 addresses; frame offsets are packed relative to obj + 0x60. */
s32 Gool_ObjectInterrupt(GoolStackObj *obj, u32 arg1, s32 argc, u32 *argv)
{
    u32 base = Ram_Addr(obj);
    s32 i;
    u32 sp;
    u32 old_fp;
    u32 new_sp;
    u32 new_fp;

    for (i = 0; i < argc; i++) {
        *RAM_AT(u32, obj->sp) = *argv++;
        obj->sp += 4;
    }
    sp = obj->sp;
    old_fp = obj->fp;
    obj->fp = sp;
    *RAM_AT(u32, obj->sp) = 0xFFFF;
    obj->sp += 4;
    *RAM_AT(u32, obj->sp) = obj->pc;
    obj->sp += 4;
    new_sp = sp - (argc + 24) * 4;
    new_fp = old_fp - 24 * 4;
    *RAM_AT(u32, obj->sp) = (new_sp - base) | ((new_fp - base) << 16);
    obj->sp += 4;
    obj->pc = ((arg1 & 0x7FFF) << 2) + RAM_AT(GoolScriptHeader, obj->script)->code;
    return GOOL_InterpretObject((struct GoolObj *)obj, 3, 0);
}

typedef struct LevelDesc {
    s32 word_0;
    s32 word_4;
    u32 eid;
    s32 path_idx;
} LevelDesc;

#define CRASH_EXT_GOOL_BROADCAST_0
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_BROADCAST_0

extern s32 gp_cur_zone;
extern s32 gp_cur_path;

#define CRASH_EXT_GOOL_BROADCAST_1
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_BROADCAST_1

/* The level descriptor at g_ns_page_pool[0]. */
#define LDAT RAM_AT(LevelDesc, g_ns_page_pool[0])

s32 Level_LdatInit(void)
{
    s32 progress;
    s32 mode;
    ZoneEntry *zone;

    g_pbak_frame_step = 0;
    if (*RAM_AT(u32, g_ns_ldat_ptr[0]) != 0x6396347F) {
        gp_level_unused_0 = 0;
        gp_level_unused_1 = 0;
        g_fade_counter[0] = 0xFFFF;
        g_cur_lid_ro[0] = 0xFFFF;
        BIOS_MemSet(g_spawns, 0, 0x4C0);
        progress = 0;
        GOOL_InitLevelSpawns(LDAT->word_4);
        mode = (g_ns_cur_level_id[0] == 0x11 || g_ns_cur_level_id[0] == 0x1E) ? 2 : 1;
        Gfx_InitMatrices();
        if (g_ns_cur_level_id[0] == 0x19 && g_game_substate == 0x200 && gp_cur_path == 0) {
            LDAT->eid = Ns_StringToEID((u8 *)gp_cur_progress);
        }
        if (gp_cur_zone == 1) {
            LDAT->eid = g_pbak_start_zone_eid[0];
            gp_cur_zone = 0;
            LDAT->path_idx = g_pbak_start_path_idx[0];
            progress = g_pbak_start_progress[0];
        }
        zone = RAM_AT(ZoneEntry, Ns_Open(&LDAT->eid, 1, mode));
        gp_next_level_ldat_eid = 0x6396347F;
        level_position.zone = 0;
        level_position.path = 0;
        level_position.progress = 0;
        g_pbak_header_byte_0[0] = 0;
        g_pbak_header_byte_1[0] = 0;
        g_pbak_header_byte_2[0] = 0;
        g_pbak_header_byte_3[0] = 0;
        g_pbak_frame_cursor[0] = engine_counter[0];
        if (g_game_substate == 0x600) {
            Pbak_Choose((s32 *)&gp_next_level_ldat_eid);
        }
        Level_Update((ResourceEntry *)zone, RAM_AT(ZonePath, zone->items[LDAT->path_idx + RAM_AT(ZoneHeader, zone->items[0])->path_index]), progress, 0);
        Ns_Close(&LDAT->eid, 1);
    } else {
        LDAT->word_0 = 0;
    }
    if (gp_next_lid == 0) {
        Shader_ParamsUpdate(1);
        Shader_ParamsUpdateRipple(1);
    }
    gp_cur_path = 0;
    return -0xFF;
}
