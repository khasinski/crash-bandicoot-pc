#include "common.h"

typedef struct {u8 reserved_00[4];u16 word04;} Context;
typedef struct {u8 reserved_00[0x2FC];u32 flags;} Data;
typedef struct {u8 reserved_00[0x10];Data *data;} Root;
#define CRASH_EXT_ENGINE_TAG_LIST
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_TAG_LIST

void Gool_ClearAllSpawns(s32 tag)
{
    u16 *entry = g_level_spawns;
    u16 *free_entry = NULL;
    tag = (tag >> 8) | (g_ns_page_pool->word04 << 9);
    if(!(g_pads->data->flags & 0x2000)) {
        while(*entry != 0) {
            if(*entry == (tag & 0xFFFF)) return;
            if(*entry == 1) free_entry = entry;
            entry++;
        }
        if(free_entry == NULL) free_entry = entry;
        *free_entry = tag;
    }
}

void Gool_RecordLevelSpawn(s32 tag)
{
    u16 *entry = g_level_spawns;
    tag = (tag >> 8) | (g_ns_page_pool->word04 << 9);
    if(g_level_spawns[0] != 0) {
        do {
            if(*entry == (tag & 0xFFFF)) {
                if(entry[1] != 0) *entry = 1;
                else *entry = 0;
                return;
            }
            entry++;
        } while(*entry != 0);
    }
}
