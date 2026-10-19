#include "common.h"
#include "ns.h"
#include "zone.h"

static __inline__ void ns_slot_unref(ResourceSlot *slot)
{
    s16 references = slot->references;
    s16 remaining;

    if (references > 0) {
        remaining = references - 1;
        slot->references = remaining;
        if ((remaining << 16) <= 0 && slot->word06 == 0) {
            Ns_PageDelete(slot);
            g_ns.pool.active_count -= 1;
        }
    }
}

static __inline__ void ns_pool_active_inc(void)
{
    g_ns.pool.active_count++;
}

s32 Ns_PageClose(s32 id, s32 count)
{
    ResourceSlot *slot;
    s32 i;

    slot = g_ns.page_map[id >> 1];
    if ((s32)slot >= -0xFE) {
        return 0;
    }
    if (slot == (ResourceSlot *)-0xFF) {
        return 1;
    }
    if ((u16)slot->word06 >= 2 && (u16)slot->word06 <= 3) {
        return 1;
    }
    for (i = 0; i < count; i++) {
        ns_slot_unref(slot);
    }
    return slot->references + 1;
}

__inline__ ResourceSlot *Ns_EntryPageStruct(s32 addr)
{
    s32 i;
    ResourceSlot *slot;
    s32 header;

    if (addr & 2) {
        return NULL;
    }
    for (i = 0; i < g_ns.physical_count; i++) {
        slot = &g_ns.physical[i];
        header = (s32)slot->header;
        if (addr >= header && addr < header + 0x10000) {
            return slot;
        }
    }
    return NULL;
}

void * Ns_EntryPage(s32 addr)
{
    ResourceSlot *slot = Ns_EntryPageStruct(addr);

    if (slot != NULL) {
        return slot->header;
    }
    return NULL;
}

void * Ns_PageStruct(u32 *ref)
{
    s32 id;
    ResourceBucket *bucket;
    s32 entry;
    ResourceSlot *slot;

    id = *ref;
    if (id & 1) {
        bucket = Ns_Probe(id);
        *ref = (u32)bucket;
    } else {
        bucket = (ResourceBucket *)id;
    }
    entry = bucket->entry;
    if (entry & 1) {
        return g_ns.page_map[entry >> 1];
    }
    slot = Ns_EntryPageStruct(entry);
    if (slot != NULL) {
        return slot;
    }
    return (ResourceSlot *)-0xFF;
}

static __inline__ ResourceSlot *ns_find_physical(s32 addr)
{
    register ResourceSlot *slot;
    register s32 i;
    s32 header;

    if (!(addr & 2)) {
        for (i = 0; i < g_ns.physical_count; i++) {
            slot = &g_ns_physical_pages[i];
            header = (s32)slot->header;
            if (addr >= header && addr < header + 0x10000) {
                return slot;
            }
        }
    }
    return NULL;
}
