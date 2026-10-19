#include "common.h"
#include "gpu.h"
#include "ns.h"

typedef ResourceEntry Entry;
typedef ResourceHeader Header;
typedef ResourceSlot Slot;
typedef ResourceBucket Bucket;
typedef ResourceHandler Handler;
extern GpuRect gp_ns_StringTable;

/* Publish a loaded page's entries. Returns the texture page index * 2, the
 * audio page's SPU address, or the page's PS1 address (tags as original). */
u32 Ns_PageUpdateEntries(s32 index) {
    Slot *request = &g_ns.physical[index], *slot;
    Header *header;
    Bucket *bucket;
    s32 i, quotient;
    Entry *entry;
    s32 (*callback)(Entry *);

    if ((u32)g_ns.wordA84 == Ram_Addr(request))
        g_ns.wordA84 = 0;
    request->state = 30;
    header = RAM_AT(Header, request->header);
    switch (header->kind) {
    case 0:
        bucket = RAM_AT(Bucket, RAM_AT(u32, g_ns.buckets)[(RAM_AT(Entry, header->entries[0])->id >> 15) & 255]);
        i = 0;
        while (i < header->count) {
            entry = RAM_AT(Entry, header->entries[i]);
            while (bucket->id != entry->id) {
                bucket++;
            }
            bucket->entry = Ram_Addr(entry);
            request->references++;
            callback = RAM_FN(s32 (*)(Entry *), g_ns_page_free_sentinel[entry->kind].callback);
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
        LoadImage((RECT *)&gp_ns_StringTable, RAM_AT(u_long, request->header));
        texture_slot->state = 20;
        texture_slot->word18 = header->id;
        texture_slot->id = request->id;
        texture_slot->mapping = Ram_Addr(Ns_Probe(texture_slot->word18));
        *RAM_AT(u32, texture_slot->mapping) = texture_slot->base;
    texture_done:
        request->state = 1;
        /* The retail code also stores the unset slot after a full table. */
        RAM_AT(u32, g_ns.page_map)[request->id >> 1] = Ram_Addr(texture_slot);
        return i * 2;
    }
    case 3:
    case 4:
        header = RAM_AT(Header, request->header);
        bucket = RAM_AT(Bucket, RAM_AT(u32, g_ns.buckets)[(RAM_AT(Entry, header->entries[0])->id >> 15) & 255]);
        slot = RAM_AT(Slot, request->slot);
        i = 0;
        while (i < header->count) {
            u32 data;
            entry = RAM_AT(Entry, header->entries[i]);
            data = entry->data;
            while (bucket->id != entry->id) {
                bucket++;
            }
            if (i == 0) {
                if (header->kind == 4)
                    g_ns_recent_page_eids[slot->word08] = bucket->id;
                else
                    g_ns_recent_page_eids[slot->word08] = 0x6396347F;
            }
            /* Audio entries record their SPU address, tagged. */
            bucket->entry = (((s32)data - (s32)request->header + (s32)slot->base) * 4) | 0x80000002;
            callback = RAM_FN(s32 (*)(Entry *), g_ns_page_free_sentinel[entry->kind].callback);
            i++;
            if (callback)
                callback(entry);
            header = RAM_AT(Header, request->header);
        }
        slot->state = 20;
        request->state = 1;
        RAM_AT(u32, g_ns.page_map)[request->id >> 1] = Ram_Addr(slot);
        return slot->base;
    }
    request->state = 20;
    return request->header;
}

