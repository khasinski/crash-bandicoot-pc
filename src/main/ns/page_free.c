#include "common.h"
#include "ns.h"

s32 Ns_PageFree(ResourceSlot *request)
{
    u8 unused[4];
    ResourceHeader *header;
    ResourceBucket *bucket;
    u32 *id_pointer;
    u32 i;
    s32 id;
    ResourceEntry *entry;
    s32 (*release)(ResourceEntry *);

    if ((u32)g_ns.wordA84 == Ram_Addr(request)) {
        g_ns.wordA84 = 0;
    }
    if ((u16)(request->state - 20) >= 2) {
        goto done;
    }
    header = RAM_AT(ResourceHeader, request->header);
    if (header->kind != 0) {
        RAM_AT(s32, g_ns.page_map)[request->id >> 1] = -0x12;
    done:
        request->state = 1;
        return -0xFF;
    }
    bucket = RAM_AT(ResourceBucket, RAM_AT(u32, g_ns.buckets)[(RAM_AT(ResourceEntry, header->entries[0])->id >> 15) & 0xFF]);
    id = request->id;
    i = 0;
    if (header->count != 0) {
        id_pointer = &bucket->id;
        do {
            entry = RAM_AT(ResourceEntry, header->entries[i]);
            while (*id_pointer != entry->id) {
                id_pointer += 2;
                bucket++;
            }
            release = RAM_FN(s32 (*)(ResourceEntry *), g_ns_page_free_sentinel[entry->kind].release);
            i++;
            if (release != NULL) {
                release(entry);
            }
            bucket->entry = id;
        } while (i < header->count);
    }
    request->state = 1;
    RAM_AT(s32, g_ns.page_map)[request->id >> 1] = -0x12;
    return -0xFF;
}

void Ns_Update(s32 arg0)
{
    s32 count;
    ResourceSlot *slot;
    s32 j;
    s32 total;
    s32 i;
    ResourceSlot *pool_slot;
    s32 best_idx;
    s32 min_id;
    s32 id;
    s32 page_count;
    s32 page_idx;
    s32 next_page;
    s32 next_id;
    s32 next_addr;

    if (g_ns.pool.active_count != 0) {
        if (g_ns.wordA84 != 0) {
            if (arg0 != -1) {
                slot = RAM_AT(ResourceSlot, g_ns.wordA84);
                total = (s32)slot->base;
                j = 0;
                slot -= total - 1;
                for (; j < total; j++, slot++) {
                    if ((s16)slot->state == 3 || (s16)slot->state == 13) {
                        Ns_PageUpdate((struct PageSlot *)slot, arg0);
                    }
                }
            }
        }
        if (g_ns.wordA84 == 0 && arg0 == -1) {
            best_idx = -1;
            for (i = 0; i < g_ns.pool.count; i++) {
                pool_slot = &g_ns.pool.slots[i];
                if ((s16)pool_slot->state == 2) {
                    id = pool_slot->id;
                    if (best_idx == -1 || id < min_id) {
                        min_id = id;
                        best_idx = i;
                    }
                }
            }
            if (best_idx != -1) {
                count = 1;
                if (gp_pause_B_pressed != 0) {
                    page_count = RAM_AT(NsDirectory, g_ns.directory)->page_count;
                    page_idx = min_id >> 1;
                    if (page_idx < page_count - 1) {
                        do {
                            next_id = page_idx + 1;
                            next_page = (next_id << 1) >> 1;
                            next_addr = RAM_AT(s32, g_ns.page_map)[next_page];
                            if (next_addr >= -0xFE || RAM_AT(ResourceSlot, next_addr)->word06 != 0) {
                                break;
                            }
                            page_idx = next_page;
                            count++;
                        } while (page_idx < page_count - 1);
                    }
                }
                Ns_PageCreatePhysical(&g_ns.pool.slots[best_idx], g_ns.pool.slots[best_idx].id, 0x6396347F, &count, 0);
            }
        }
        for (i = 0; i < g_ns.physical_count; i++) {
            Ns_PageUpdate((struct PageSlot *)&g_ns.physical[i], arg0);
        }
    }
}
