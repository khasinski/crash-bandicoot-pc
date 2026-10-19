#include "resource.h"

/* Find an eight-byte entry in the ID hash bucket. */

ResourceBucket *Ns_Probe(s32 id) {
    ResourceBucket *entry = g_ns_hash_buckets[((u32)id >> 15) & 0xFF];
    while (entry->id != id) {
        entry++;
    }
    return entry;
}
