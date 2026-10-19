#include "common.h"
#include "gpu.h"

typedef struct WgeoHeader {
    s32 x;
    s32 y;
    s32 z;
    s32 word_0c;
    s32 word_10;
    s32 word_14;
    u32 tpage_count;
    s32 flags;
    u32 tpages[8];
} WgeoHeader;

typedef struct ZoneWorld {
    u32 eid;
    s32 trans[3];
    RAM_REF(WgeoHeader) wgeo;
    RAM_REF(void) item1;
    RAM_REF(void) item2;
    RAM_REF(void) polys;
    RAM_REF(void) tpages[8];
} ZoneWorld;

typedef struct ZoneHeader9A2C {
    s32 world_count;
    ZoneWorld worlds[8];
} ZoneHeader9A2C;

typedef struct ResourceItem {
    u8 reserved_00[0x10];
    RAM_REF(void) item0;
    RAM_REF(void) item1;
    RAM_REF(void) item2;
} ResourceItem;

#define CRASH_EXT_RENDER_WORLD_SETUP
#include "render_externs.h"
#undef CRASH_EXT_RENDER_WORLD_SETUP

void Gfx_LoadWorldsFog(ZoneHeader9A2C *arg0)
{
    s32 v[3];
    s32 i;
    u32 j;
    ResourceItem *entry;
    WgeoHeader *wgeo;
    s32 *trans;

    for (i = 0; i < arg0->world_count; i++) {
        entry = RAM_AT(ResourceItem, Ns_Lookup(&arg0->worlds[i].eid));
        wgeo = RAM_AT(WgeoHeader, entry->item0);
        if (wgeo->flags == 1) {
            arg0->worlds[i].trans[0] = 0;
            arg0->worlds[i].trans[1] = 0;
            arg0->worlds[i].trans[2] = 0;
        } else {
            trans = (s32 *)((u8 *)arg0 + i * 64);
            trans += 2;
            trans[0] = (g_m_light * (v[0] = wgeo->x - (g_cam_trans_x >> 8))
                      + g_m_light_01 * (v[1] = wgeo->y - (g_cam_trans_y >> 8))
                      + g_m_light_02 * (v[2] = wgeo->z - (g_cam_trans_z >> 8))) >> 12;
            trans[1] = (g_m_light_10 * v[0]
                      + g_m_light_11 * v[1]
                      + g_m_light_12 * v[2]) >> 12;
            trans[2] = (g_m_light_20 * v[0]
                      + g_m_light_21 * v[1]
                      + g_m_light_22 * v[2]) >> 12;
        }
        arg0->worlds[i].wgeo = Ram_Addr(wgeo);
        arg0->worlds[i].item1 = entry->item1;
        arg0->worlds[i].item2 = entry->item2;
        arg0->worlds[i].polys = Ram_Addr(wgeo + 1);
        for (j = 0; j < wgeo->tpage_count; j++) {
            arg0->worlds[i].tpages[j] = Ns_Lookup(&wgeo->tpages[j]);
        }
    }
    if (RAM_AT(WorldSetupLevel, g_ns_page_pool)->level_id == 0x19 && g_island_cam_state == 0xF) {
        Gfx_FreezeMapTextures(g_gem_count, g_title_fog_freeze_flag);
    }
}
