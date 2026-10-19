#include "common.h"
#include "gpu.h"
#include "ns.h"

typedef ResourceEntry Entry;
typedef ResourceHeader Header;
typedef ResourceSlot Slot;
typedef ResourceBucket Bucket;
typedef ResourceHandler Handler;
GpuRect gp_ns_StringTable = { 0, 0, 0x100, 0x80 };

u8 *Ns_PageUpdateEntries(s32 index) {
    Slot *request = &g_ns.physical[index], *slot;
    Header *header;
    Bucket *bucket;
    s32 i, quotient;
    Entry *entry;
    s32 (*callback)(Entry *);

    if ((Slot *)g_ns.wordA84 == request)
        g_ns.wordA84 = 0;
    request->state = 30;
    header = request->header;
    switch (header->kind) {
    case 0:
        bucket = g_ns.buckets[(header->entries[0]->id >> 15) & 255];
        i = 0;
        while (i < request->header->count) {
            entry = request->header->entries[i];
            while (bucket->id != entry->id) {
                bucket++;
            }
            bucket->entry = (u32)entry;
            request->references++;
            callback = g_ns_page_free_sentinel[entry->kind].callback;
            i++;
            if (callback)
                callback(entry);
            request->references--;
        }
        break;
    case 1: {
        Slot *texture_slot;
        i = Ns_TexturePageAllocate();
        if (i == -12) {
            i = 15;
            if (request->references == 0)
                goto texture_done;
        }
        texture_slot = &g_slst_buf0[i];
        quotient = i / 4;
        gp_ns_StringTable.x = (i - quotient * 4) << 8;
        gp_ns_StringTable.y = quotient << 7;
        LoadImage(&gp_ns_StringTable, request->header);
        texture_slot->state = 20;
        texture_slot->word18 = request->header->id;
        texture_slot->id = request->id;
        texture_slot->mapping = (u32 *)Ns_Probe(texture_slot->word18);
        *texture_slot->mapping = (u32)texture_slot->base;
    texture_done:
        request->state = 1;
        g_ns.page_map[request->id >> 1] = texture_slot;
        return (u8 *)(i * 2);
    }
    case 3:
    case 4:
        header = request->header;
        bucket = g_ns.buckets[(header->entries[0]->id >> 15) & 255];
        slot = request->slot;
        i = 0;
        while (i < header->count) {
            u8 *data;
            entry = header->entries[i];
            data = entry->data;
            while (bucket->id != entry->id) {
                bucket++;
            }
            if (i == 0) {
                if (request->header->kind == 4)
                    g_ns_recent_page_eids[slot->word08] = bucket->id;
                else
                    g_ns_recent_page_eids[slot->word08] = 0x6396347F;
            }
            bucket->entry = (u32)((((s32)data - (s32)request->header + (s32)slot->base) * 4) | 0x80000002);
            callback = g_ns_page_free_sentinel[entry->kind].callback;
            i++;
            if (callback)
                callback(entry);
            header = request->header;
        }
        slot->state = 20;
        request->state = 1;
        g_ns.page_map[request->id >> 1] = slot;
        return slot->base;
    }
    request->state = 20;
    return (u8 *)request->header;
}

