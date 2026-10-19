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

    message.sender = arg0;
    message.word04 = arg2;
    message.word0C = arg3;
    message.word10 = 0;
    message.word08 = arg4;
    message.word14 = arg5;
    if (arg1->kind == 2) {
        node = arg1->word04;
    } else {
        node = arg1->children;
    }
    if (node != NULL) {
        do {
            child = node->children;
            next = node->next;
            if (child != NULL) {
                do {
                    next_child = child->next;
                    if (GOOL_TraverseObjectTreePostorder(child, Gool_SendIfColliding, context) == -25) {
                        Gool_ObjectKill(node, 0);
                    }
                    child = next_child;
                } while (child != NULL);
            }
            if (Gool_SendIfColliding(node, context) == -25) {
                Gool_ObjectKill(arg1, 0);
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

s32 Gool_ObjectInterrupt(GoolStackObj *obj, u32 arg1, s32 argc, u32 *argv)
{
    s32 i;
    u32 *sp;
    u32 *old_fp;
    u32 *new_sp;
    u32 *new_fp;

    for (i = 0; i < argc; i++) {
        *obj->sp++ = *argv++;
    }
    sp = obj->sp;
    old_fp = obj->fp;
    obj->fp = sp;
    *obj->sp++ = 0xFFFF;
    *obj->sp++ = (u32)obj->pc;
    *obj->sp++ = ((u32)(new_sp = sp - argc - 24) - (u32)obj) | (((u32)(new_fp = old_fp - 24) - (u32)obj) << 16);
    obj->pc = ((arg1 & 0x7FFF) << 2) + obj->script->code;
    return GOOL_InterpretObject(obj, 3, 0);
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

s32 Level_LdatInit(void)
{
    s32 progress;
    s32 mode;
    ZoneEntry *zone;

    g_pbak_frame_step = 0;
    if (*g_ns_ldat_ptr[0] != 0x6396347F) {
        gp_level_unused_0 = 0;
        gp_level_unused_1 = 0;
        g_fade_counter[0] = 0xFFFF;
        g_cur_lid_ro[0] = 0xFFFF;
        BIOS_MemSet(g_spawns, 0, 0x4C0);
        progress = 0;
        GOOL_InitLevelSpawns(g_ns_page_pool[0]->word_4);
        mode = (g_ns_cur_level_id[0] == 0x11 || g_ns_cur_level_id[0] == 0x1E) ? 2 : 1;
        Gfx_InitMatrices();
        if (g_ns_cur_level_id[0] == 0x19 && g_game_substate == 0x200 && gp_cur_path == 0) {
            g_ns_page_pool[0]->eid = Ns_StringToEID(gp_cur_progress);
        }
        if (gp_cur_zone == 1) {
            g_ns_page_pool[0]->eid = g_pbak_start_zone_eid[0];
            gp_cur_zone = 0;
            g_ns_page_pool[0]->path_idx = g_pbak_start_path_idx[0];
            progress = g_pbak_start_progress[0];
        }
        zone = (ZoneEntry *)Ns_Open(&g_ns_page_pool[0]->eid, 1, mode);
        gp_next_level_ldat_eid = 0x6396347F;
        level_position.zone = NULL;
        level_position.path = NULL;
        level_position.progress = 0;
        g_pbak_header_byte_0[0] = 0;
        g_pbak_header_byte_1[0] = 0;
        g_pbak_header_byte_2[0] = 0;
        g_pbak_header_byte_3[0] = 0;
        g_pbak_frame_cursor[0] = engine_counter[0];
        if (g_game_substate == 0x600) {
            Pbak_Choose(&gp_next_level_ldat_eid);
        }
        Level_Update(zone, zone->items[g_ns_page_pool[0]->path_idx + ((ZoneHeader *)zone->items[0])->path_index], progress, 0);
        Ns_Close(&g_ns_page_pool[0]->eid, 1);
    } else {
        g_ns_page_pool[0]->word_0 = 0;
    }
    if (gp_next_lid == 0) {
        Shader_ParamsUpdate(1);
        Shader_ParamsUpdateRipple(1);
    }
    gp_cur_path = 0;
    return -0xFF;
}
