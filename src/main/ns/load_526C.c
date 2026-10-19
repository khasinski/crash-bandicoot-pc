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
            } else if (RAM_AT(ResourceBucket, val)->id == entry_id) {
            found:
                return 1;
            }
            i++;
        }
        for (i = 0; i < load->page_count; i++) {
            if (load->pages[i] == (u32)slot->id) {
                return 1;
            }
        }
        return 0;
    }
    return 0;
}

#define NS_FIND_OR_ALLOC_SLOT(res_slot, page_id, ent_id, found_label, search_label, done_label) \
    do { \
        ResourceSlot *_cand; \
        s32 _i; \
        u32 _ticks; \
        (res_slot) = RAM_AT(s32, g_ns.page_map)[(page_id) >> 1]; \
        if ((res_slot) == -0x12) { \
            goto search_label; \
        } \
        if ((res_slot) != -0xFF) { \
            RAM_AT(ResourceSlot, res_slot)->word18 = (ent_id); \
        } \
        goto done_label; \
    found_label: \
        _ticks = frame_counter; \
        _cand->state = 2; \
        _cand->id = (page_id); \
        _cand->references = 0; \
        _cand->word18 = (ent_id); \
        _cand->word28 = _ticks; \
        RAM_AT(u32, g_ns.page_map)[(page_id) >> 1] = Ram_Addr(_cand); \
        ns_pool_active_inc(); \
        (res_slot) = (s32)Ram_Addr(_cand); \
        goto done_label; \
    search_label: \
        for (_i = 0; _i < g_ns_virtual_page_pool.count; _i++) { \
            _cand = &g_ns_virtual_pages[_i]; \
            if ((s16)_cand->state == 1) { \
                goto found_label; \
            } \
        } \
        (res_slot) = 0; \
    done_label:; \
    } while (0)

/* Returns the virtual slot's PS1 address, -0xFF, or 0 when none is free. */
s32 Ns_PageVirtual(s32 id, s32 entry_id)
{
    s32 slot;

    NS_FIND_OR_ALLOC_SLOT(slot, id, entry_id, found_0, search_0, done_0);
    return slot;
}

