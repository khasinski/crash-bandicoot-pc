#include "resource.h"

/* Hash buckets point into eight-byte resource entries. */
typedef ResourceBucket Entry;
typedef ResourceHashMetadata Metadata;
Entry *Ns_ProbeSafe(u32 id) {
    Entry *cursor = g_ns_hash_buckets[(id >> 15) & 255];
    if (cursor->id == id) goto done;
    {
    Metadata *metadata = g_ns_nsd_header;
    Entry *base = g_ns_entry_table;
    s32 count = metadata->count;
    cursor++;
search:
    if (((s32)((u8 *)cursor - (u8 *)base) >> 3) >= count) return (Entry *)-10;
    {
        u32 candidate = cursor->id;
        cursor++;
        if (candidate != id) goto search;
    }
    cursor--;
    }
done:
    return cursor;
}
