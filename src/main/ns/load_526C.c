#include "common.h"
#include "ns.h"
#include "zone.h"

__inline__ s32 Ns_PageClose(s32 id, s32 count);

static __inline__ void ns_pool_active_inc(void)
{
    g_ns.pool.active_count++;
}

s32 Ns_PageCommitVirtual(ZoneLoadList *load, ResourceSlot *slot)
{
    s32 i;
    u32 *ref;
    u32 entry_id;
    u32 val;

    if (load != NULL) {
        i = 0;
        while (i < load->entry_count) {
            ref = &load->entries[i];
            entry_id = slot->word18;
            val = *ref;
            if (val & 1) {
                if (val == entry_id) {
                    goto found;
                }
            } else if (((ResourceBucket *)val)->id == entry_id) {
            found:
                return 1;
            }
            i++;
        }
        {
            s32 ret;
            for (i = 0; i < load->page_count; i++) {
                if (load->pages[i] == (u32)slot->id) {
                    ret = 1;
                    goto found2;
                }
            }
            {
                s32 z = i & 1;
                if ((z >> 1) == 0) {
                    goto ret0;
                }
            }
            ret = 0;
        found2:
            return ret;
        }
    ret0:
        return 0;
    }
    return 0;
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

ResourceSlot *Ns_PageVirtual(s32 id, s32 entry_id)
{
    ResourceSlot *slot;

    NS_FIND_OR_ALLOC_SLOT(slot, id, entry_id, found_0, search_0, done_0);
    return slot;
}

