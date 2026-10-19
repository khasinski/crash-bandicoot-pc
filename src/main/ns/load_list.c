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

void Ns_ZoneUnload(ZoneLoadList *load)
{
    s32 i;
    s32 slot;

    if (load != NULL) {
        for (i = 0; i < load->entry_count; i++) {
            slot = Ns_PageStruct(&load->entries[i]);
            if (slot < -0xFE && slot != -0xFF) {
                ns_slot_unref(RAM_AT(ResourceSlot, slot));
            }
        }
        for (i = 0; i < load->page_count; i++) {
            Ns_PageClose(load->pages[i], 1);
        }
    }
}
