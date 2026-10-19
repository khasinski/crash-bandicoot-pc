#include "resource.h"

/* Hash buckets point into eight-byte resource entries. */
typedef ResourceBucket Entry;
typedef ResourceHashMetadata Metadata;
/* Returns the entry's PS1 address, or -10 when the ID is absent. */
s32 Ns_ProbeSafe(u32 id) {
    Entry *cursor = RAM_AT(Entry, RAM_AT(u32, g_ns_hash_buckets)[(id >> 15) & 255]);
    if (cursor->id == id) goto done;
    {
    Metadata *metadata = RAM_AT(Metadata, g_ns_nsd_header);
    Entry *base = RAM_AT(Entry, g_ns_entry_table);
    s32 count = metadata->count;
    cursor++;
search:
    if (((s32)((u8 *)cursor - (u8 *)base) >> 3) >= count) return -10;
    {
        u32 candidate = cursor->id;
        cursor++;
        if (candidate != id) goto search;
    }
    cursor--;
    }
done:
    return (s32)Ram_Addr(cursor);
}
