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

#define NS_ADD_REF(page_id, is_sync, ref_delta, ent_id, found_lbl, search_lbl, done_lbl) \
    { \
        s32 _id = (page_id); \
        s32 _entry_id = (ent_id); \
        ResourceSlot *_slot; \
        if ((is_sync) != 0) { \
            _slot = Ns_PagePhysical(_id, _entry_id); \
        } else { \
            ResourceSlot *_inlined_slot; \
            NS_FIND_OR_ALLOC_SLOT(_inlined_slot, _id, _entry_id, found_lbl, search_lbl, done_lbl); \
            _slot = _inlined_slot; \
        } \
        if (_slot != (ResourceSlot *)-0xFF) { \
            _slot->references += (ref_delta); \
        } \
    }

void * Ns_Open(u32 *ref, s32 sync, s32 delta)
{
    s32 id;
    ResourceBucket *bucket;
    s32 entry;
    ResourceSlot *slot;

    id = *ref;
    if ((u32)id == 0x6396347FU) {
        return (ResourceEntry *)-0xA;
    }
    if (id & 1) {
        *ref = (u32)Ns_Probe(id);
        bucket = (ResourceBucket *)*ref;
    } else {
        bucket = (ResourceBucket *)id;
    }
    entry = bucket->entry;
    if (!(entry & 1)) {
        if (delta != 0) {
            s32 i;
            ResourceSlot *phys_slot;
            s32 header;
            s32 addr = entry;

            if (!(addr & 2)) {
                for (i = 0; i < g_ns.physical_count; i++) {
                    phys_slot = &g_ns_physical_pages[i];
                    header = (s32)phys_slot->header;
                    if (addr >= header && addr < header + 0x10000) {
                        slot = phys_slot;
                        goto found_phys;
                    }
                }
            }
            slot = NULL;
        found_phys:
            if (slot != NULL) {
                NS_ADD_REF(slot->id, sync, delta, ((ResourceEntry *)bucket->entry)->id, found_2, search_2, done_2);
            }
        }
        return (ResourceEntry *)bucket->entry;
    }
    NS_ADD_REF(entry, sync, delta, bucket->id, found_3, search_3, done_3);
    if (!(bucket->entry & 1)) {
        return (ResourceEntry *)bucket->entry;
    }
    return NULL;
}
