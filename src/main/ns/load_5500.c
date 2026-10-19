#include "common.h"
#include "ns.h"
#include "zone.h"

__inline__ s32 Ns_PageClose(s32 id, s32 count);

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

s32 Ns_PageOpen(s32 id, s32 sync, s32 delta, s32 entry_id)
{
    ResourceSlot *slot;

    if (sync != 0) {
        slot = Ns_PagePhysical(id, entry_id);
    } else {
        ResourceSlot *inlined_slot;
        NS_FIND_OR_ALLOC_SLOT(inlined_slot, id, entry_id, found_1, search_1, done_1);
        slot = inlined_slot;
    }
    if (slot == (ResourceSlot *)-0xFF) {
        return 1;
    }
    return slot->references += delta;
}

