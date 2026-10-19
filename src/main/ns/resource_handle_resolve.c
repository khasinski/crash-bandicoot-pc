#include "common.h"
#include "resource.h"
NsItemList *Ns_Lookup(u32 *reference) {
    u32 id = *reference;
    ResourceHandle *resolved;
    u32 value;
    s32 result;
    if (id & 1) {
        ResourceHandle *slot,*found;
        u32 key = id;
        slot = *(ResourceHandle **)(((id >> 13) & 0x3FC) + (u32)g_ns_hash_buckets);
        if (slot->id != id) {
            u32 candidate;
            found = slot + 1;
            do {
                candidate = found->id;
                found++;
            } while (candidate != key);
            slot = found - 1;
        }
        *reference = (u32)slot;
        resolved = slot;
    } else {
        resolved = (ResourceHandle *)id;
    }
    value = resolved->entry;
    result = value;
    if (value & 1) {
        Ns_PagePhysical(value,resolved->id);
        value = resolved->entry;
        result = -10;
        if (!(value & 1)) {
            result = value;
        }
    }
    return (NsItemList *)result;
}
