#include "common.h"
#include "resource.h"

/* Prefer free or inactive slots; otherwise evict a texture absent from the
 * current zone load list. Odd references are IDs, even references are entries.
 */
s32 Ns_TexturePageAllocate(void)
{
    s32 i;
    s32 j;
    s32 used;
    s32 offset;
    s32 wanted;
    u32 bias;
    ResourceSlot *slot;
    ResourceSlot *scan_slot;
    s32 scan_offset;
    ZoneLoadInfo *zone;
    u32 ref;
    u32 id;
    i = 15;
    wanted = 1;
    scan_slot = &g_slst_buf0[15];
    do
    {
        if (((s16) scan_slot->state) == wanted)
            goto found;
        i--;
        scan_slot--;
    }
    while (i >= 0);
    i = 15;
    wanted = 21;
    scan_offset = 15 * 44;
    do
    {
        if ((*((s16 *) ((u8 *) g_slst_buf1 + scan_offset))) == wanted)
            goto found;
        i--;
        scan_offset -= 44;
    }
    while (i >= 0);
    i = 15;
    if (g_pads != 0)
        goto search;
    Ns_TexturePageFree(15);
    return 15;
    recycle:
    Ns_TexturePageFree(i);

    found:
    return i;

    search:
    bias = 0xFFEC;

    zone = RAM_AT(ZoneLoadInfo, RAM_AT(ResourceEntry, g_pads)->data);
    offset = 15 * 44;
    slot = &g_slst_buf0[15];
    do
    {
        used = 0;
        if (((u16) (slot->state + bias)) < 2)
        {
            for (j = 0; j < zone->count; j++)
            {
                id = *((u32 *) ((u8 *) g_ns_tex_hash_table + offset));
                ref = zone->refs[j];
                if (ref & 1)
                {
                    if (ref == id)
                        goto hit;
                }
                else
                    if (RAM_AT(ResourceEntry, ref)->id == id)
                    goto hit;
                continue;
                hit:
                used = 1;

                break;
            }

            if (!used)
                goto recycle;
        }
        offset -= 44;
        i--;
        slot--;
    }
    while (i >= 0);
    return -12;
}
