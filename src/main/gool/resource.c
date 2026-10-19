#include "common.h"

/* Initialise resource state on first use; visit object roots and clear flags on release. */

#include "gool.h"
#include "zone.h"

#define CRASH_EXT_GOOL_RESOURCE
#include "gool_externs.h"
#undef CRASH_EXT_GOOL_RESOURCE

s32 Level_ZdatOnLoad(ZoneEntry *zone)
{
    ZoneHeader *header = RAM_AT(ZoneHeader, zone->items[0]);
    s32 i;

    for (i = 0; i < header->path_count; i++) {
        RAM_AT(ZonePath, zone->items[i + header->path_index])->zone = Ram_Addr(zone);
    }
    for (i = 0; i < header->entity_count; i++) {
        *RAM_AT(u32, zone->items[i + header->path_index + header->path_count]) = Ram_Addr(zone);
    }
    return -0xFF;
}

s32 Level_MdatOnLoad(ZoneEntry *zone)
{
    s32 i;

    for (i = 0; i < RAM_AT(ResourceMapMetadata, zone->items[0])->count; i++) {
        *RAM_AT(u32, zone->items[i + 2]) = Ram_Addr(zone);
    }
    return -0xFF;
}

s32 Level_ZdatOnLoad2(ZoneEntry *zone, ZoneEntry *neighbor)
{
    ZoneHeader *header = RAM_AT(ZoneHeader, zone->items[0]);
    s32 i;

    for (i = 0; i < header->neighbor_count; i++) {
        if ((u32)Ns_Lookup(&header->neighbors[i]) == Ram_Addr(neighbor)) {
            return 1;
        }
    }
    return 0;
}

void Level_MdatOnLoad2(ObjectView *object)
{
    u8 *resource = RAM_AT(u8, object->resource);
    if (!(*(u32 *)(resource + CRASH_OFFSETOF(GoolResourceState, flags2DC)) & 1)) {
        g_doctor_cortex_obj[0] = 0;
        g_mdat_title_str_ptr = 0;
        g_spu_reverb_addr[0] = 0x19000;
        *(u32 *)(resource + CRASH_OFFSETOF(GoolResourceState, flags2DC)) |= 3;
    }
}

void Zone_TerminateObjects(ObjectView *object)
{
    u8 *resource = RAM_AT(u8, object->resource);
    if (*(u32 *)(resource + CRASH_OFFSETOF(GoolResourceState, flags2DC)) & 1) {
        Gool_ZoneObjectsTerminate(object);
        *(u32 *)(resource + CRASH_OFFSETOF(GoolResourceState, flags2DC)) &= ~3;
    }
}
