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

static __inline__ s32 Ns_PageClose_inline(s32 id, s32 count)
{
    ResourceSlot *slot;
    s32 addr;
    s32 i;

    addr = RAM_AT(s32, g_ns.page_map)[id >> 1];
    if (addr >= -0xFE) {
        return 0;
    }
    if (addr == -0xFF) {
        return 1;
    }
    slot = RAM_AT(ResourceSlot, addr);
    if ((u16)slot->word06 >= 2 && (u16)slot->word06 <= 3) {
        return 1;
    }
    for (i = 0; i < count; i++) {
        ns_slot_unref(slot);
    }
    return slot->references + 1;
}

static __inline__ ResourceSlot *Ns_EntryPageStruct(s32 addr)
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

s32 Ns_Close(u32 *ref, s32 count)
{
    s32 id;
    ResourceBucket *bucket;
    s32 entry;
    ResourceSlot *slot;

    id = *ref;
    if (id & 1) {
        bucket = Ns_Probe(id);
        *ref = Ram_Addr(bucket);
    } else {
        bucket = RAM_AT(ResourceBucket, id);
    }
    entry = bucket->entry;
    if (!(entry & 1)) {
        if (count == 0) {
        ret_1:
            return 1;
        }
        slot = Ns_EntryPageStruct(entry);
        if (slot == NULL) {
            goto ret_1;
        }
        return Ns_PageClose_inline(slot->id, count);
    }
    if (count == 0) {
        return 0;
    }
    return Ns_PageClose_inline(entry, count);
}

s32 Ns_CountAvailablePages(void)
{
    s32 free_count;
    s32 i;
    ResourceSlot *slot;

    free_count = g_ns.physical_count;
    for (i = 0; i < g_ns.pool.count; i++) {
        slot = &g_ns.pool.slots[i];
        if ((s16)slot->state != 1 && slot->references != 0) {
            free_count--;
        }
    }
    for (i = 0; i < g_ns.physical_count; i++) {
        slot = &g_ns.physical[i];
        if ((s16)slot->state != 1 && slot->references != 0) {
            free_count--;
        }
    }
    return free_count;
}

/* refs holds the PS1 addresses of count reference words (a GOOL stack span). */
s32 Ns_CountAvailablePages2(u32 *refs, s32 count)
{
    s32 seen_pages[40];
    s32 total = 0;
    s32 seen_count = 0;
    s32 i;
    s32 j;
    s32 id;
    ResourceBucket *bucket;
    s32 entry;
    ResourceSlot *phys_slot;
    s32 page_slot;
    u32 *ref;

    for (i = 0; i < count; i++) {
        ref = RAM_AT(u32, refs[i]);
        id = *ref;
        if (id & 1) {
            bucket = Ns_Probe(id);
            *ref = Ram_Addr(bucket);
        } else {
            bucket = RAM_AT(ResourceBucket, id);
        }
        entry = bucket->entry;
        if (!(entry & 1)) {
            phys_slot = Ns_EntryPageStruct(entry);
            if (phys_slot != NULL) {
                if ((s16)phys_slot->state == 1 || phys_slot->references == 0) {
                    total++;
                }
            }
        } else {
            page_slot = RAM_AT(s32, g_ns.page_map)[entry >> 1];
            if (page_slot == -0x12) {
                for (j = 0; j < seen_count; j++) {
                    if (entry == seen_pages[j]) {
                        break;
                    }
                }
                if (j == seen_count) {
                    total++;
                    seen_pages[seen_count++] = bucket->entry;
                }
            } else if (page_slot != -0xFF) {
                if (RAM_AT(ResourceSlot, page_slot)->references == 0) {
                    total++;
                }
            }
        }
    }
    return total;
}
