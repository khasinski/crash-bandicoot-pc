#include "common.h"
#include "resource.h"
/* Resolve a reference and page its entry in synchronously. Returns the
 * entry's PS1 address or -10; an EID reference is resolved in place. */
s32 Ns_Lookup(u32 *reference) {
    u32 id = *reference;
    ResourceHandle *resolved;
    u32 value;
    s32 result;
    if (id & 1) {
        ResourceHandle *slot,*found;
        u32 key = id;
        slot = RAM_AT(ResourceHandle, RAM_AT(u32, g_ns_hash_buckets)[(id >> 15) & 0xFF]);
        if (slot->id != id) {
            u32 candidate;
            found = slot + 1;
            do {
                candidate = found->id;
                found++;
            } while (candidate != key);
            slot = found - 1;
        }
        *reference = Ram_Addr(slot);
        resolved = slot;
    } else {
        resolved = RAM_AT(ResourceHandle, id);
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
    return result;
}
