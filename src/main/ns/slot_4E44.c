#include "common.h"
#include "ns.h"

ResourceSlot *Ns_PageCreateVirtual(s32 id, s32 word18)
{
    s32 i;
    ResourceSlot *slot;

    for (i = 0; i < g_ns.pool.count; i++) {
        slot = &g_ns.pool.slots[i];
        if ((s16)slot->state == 1) {
            slot->state = 2;
            slot->id = id;
            slot->references = 0;
            slot->word18 = word18;
            slot->word28 = frame_counter;
            RAM_AT(u32, g_ns.page_map)[id >> 1] = Ram_Addr(slot);
            g_ns.pool.active_count++;
            return slot;
        }
    }
    return NULL;
}

/* Returns the head physical slot's PS1 address, or -12 when no pages are free. */
s32 Ns_PageCreatePhysical(ResourceSlot *slot, s32 id, s32 word18, s32 *count_ptr, s32 arg4)
{
    s32 index;
    s32 count;
    ResourceSlot *phys;
    ResourceSlot *head;
    s32 i;
    s32 base_id;

    index = Ns_PageAllocate(count_ptr);
    if (index == -1) {
        return -12;
    }
    count = *count_ptr;
    phys = &g_ns.physical[index + count - 1];
    if (slot != NULL) {
        i = count - 1;
        base_id = slot->id;
        head = phys;
        if (i >= 0) {
            do {
                slot = RAM_AT(ResourceSlot, RAM_AT(u32, g_ns.page_map)[(base_id >> 1) + i]);
                phys->id = slot->id;
                if (word18 != 0x6396347F) {
                    phys->word18 = word18;
                } else {
                    phys->word18 = slot->word18;
                }
                slot->state = 1;
                phys->references = slot->references;
                phys->word28 = slot->word28;
                phys->base = *count_ptr;
                phys->slot = Ram_Addr(head);
                if (i == *count_ptr - 1) {
                    phys->state = 2;
                } else {
                    phys->state = (count = 12);
                }
                RAM_AT(u32, g_ns.page_map)[phys->id >> 1] = Ram_Addr(phys);
                phys--;
                i--;
            } while (i >= 0);
        }
        return (s32)Ram_Addr(head);
    }
    *count_ptr = 1;
    phys->id = id;
    phys->word18 = word18;
    phys->references = 0;
    phys->word28 = frame_counter;
    g_ns.pool.active_count++;
    phys->base = 1;
    phys->slot = Ram_Addr(phys);
    phys->state = 2;
    RAM_AT(u32, g_ns.page_map)[phys->id >> 1] = Ram_Addr(phys);
    return (s32)Ram_Addr(phys);
}
