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

#define NS_FIND_OR_ALLOC_SLOT(res_slot, page_id, ent_id, found_label, search_label, done_label) \
    do { \
        ResourceSlot *_cand; \
        s32 _i; \
        u32 _ticks; \
        (res_slot) = g_ns.page_map[(page_id) >> 1]; \
        if ((res_slot) == (ResourceSlot *)-0x12) { \
            goto search_label; \
        } \
        if ((res_slot) != (ResourceSlot *)-0xFF) { \
            (res_slot)->word18 = (ent_id); \
        } \
        goto done_label; \
    found_label: \
        _ticks = frame_counter; \
        _cand->state = 2; \
        _cand->id = (page_id); \
        _cand->references = 0; \
        _cand->word18 = (ent_id); \
        _cand->word28 = _ticks; \
        g_ns.page_map[(page_id) >> 1] = _cand; \
        ns_pool_active_inc(); \
        (res_slot) = _cand; \
        goto done_label; \
    search_label: \
        for (_i = 0; _i < g_ns_virtual_page_pool.count; _i++) { \
            _cand = &g_ns_virtual_pages[_i]; \
            if ((s16)_cand->state == 1) { \
                goto found_label; \
            } \
        } \
        (res_slot) = NULL; \
    done_label:; \
    } while (0)

void * Ns_PagePhysical(s32 id, s32 entry_id)
{
    ResourceSlot *slot;
    s32 sp18;
    s32 sp1C;

    slot = g_ns.page_map[id >> 1];
    if ((s32)slot >= -0xFE) {
        sp18 = 1;
        slot = Ns_PageCreatePhysical(NULL, id, entry_id, &sp18, 1);
        if ((s32)slot < -0xFE) {
            goto sync_page;
        }
        return slot;
    }
    if (slot == (ResourceSlot *)-0xFF || ((u16)slot->word06 >= 2 && (u16)slot->word06 <= 3)) {
        return slot;
    }
    if (slot->word06 == 0) {
        sp1C = 1;
        slot = Ns_PageCreatePhysical(slot, id, entry_id, &sp1C, 1);
        if ((s32)slot < -0xFE) {
            goto sync_page;
        }
        return slot;
    }
    slot->word18 = entry_id;
    if ((s16)slot->state != 0x14) {
    sync_page:
        Ns_Update(id);
        slot = g_ns.page_map[id >> 1];
    }
    return slot;
}
