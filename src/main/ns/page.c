#include "common.h"
#include "ns.h"

/* Return an item span from adjacent offsets in a nonempty resource page. */

s32 Ns_VirtualPagePhysicalIdx(ResourceEntry *entry)
{
    ResourceHeader *page = (ResourceHeader *)entry;
    s32 *item;
    s32 size;

    switch (((ResourceHeader *)entry)->kind) {
    case 1:
        return 0x10000;
    case 0:
    case 3:
    case 4:
        item = (s32 *)((u8 *)page + (page->count << 2));
    common:
        /* Translated pages hold PS1 addresses, which are negative as s32. */
        size = item[4];
        if (size < 0) {
            return size - (s32)Ram_Addr(page);
        }
        return size;
    case 0x100:
        item = (s32 *)((u8 *)entry + (entry->word0C << 2));
        goto common;
    default:
        return -14;
    }
}

s32 Ns_EntryItemSize(s32 *page, s32 index)
{
    s32 *entry;
    s32 result;
    if (!page[3]) {
        result = 0;
    } else {
        entry = page + index;
        result = entry[5] - entry[4];
    }
    return result;
}

/* Returns the buffer's PS1 address, or a negative error code. */
s32 Ns_ReadSectors(void *arg0, void *buffer, s32 read_full)
{
    u8 result[8];
    CdlLOC position;
    NsFileMap *map;

    if (gp_pause_B_pressed) {
        if (read_full == 1) {
            map = (NsFileMap *)arg0;
            if (buffer == NULL) {
                buffer = LIBC_Malloc(((map->directory_size & 0x1FFFF) + 0x7FF) / 0x800 * 0x800);
                if (buffer == NULL) {
                    return -15;
                }
            }
            CdIntToPos(map->directory_sector & 0x7FFFFF, &position);
            CdControl(2, (u_char *)&position, result);
            if (!CdRead(((map->directory_size & 0x1FFFF) + 0x7FF) / 0x800, buffer, 0x80)
                || CdReadSync(0, 0)) {
                return -16;
            }
            buffer = LIBC_Realloc(buffer, map->directory_size & 0x1FFFF);
        }
        return (s32)Ram_Addr(buffer);
    }
    /* Without CD access the original returned the zero flag it tested. */
    return 0;
}

s32 Ns_FileOpen(NsState *ns, s32 type)
{
    s32 i;
    s32 count = 0;

    NsDirectory *directory = RAM_AT(NsDirectory, ns->directory);

    for (i = 0; i < directory->entry_count; i++) {
        if (Ns_EIDType(&directory->entries[i]) == type) {
            count++;
        }
    }
    return count;
}

s32 Ns_PageTranslateOffsets(ResourceHeader *page)
{
    s32 count = page->count;
    u32 *entries = page->entries;
    u32 base = Ram_Addr(page);
    s32 i, j;
    ResourceEntry *entry;
    u32 *items;

    if (page->kind == 1) {
        return -0xFF;
    }
    if (page->word00 != 0x1234) {
        return -0x12;
    }
    for (i = count - 1; i >= 0; i--) {
        entries[i] += base;
        entry = RAM_AT(ResourceEntry, entries[i]);
        items = (u32 *)&entry->data;
        for (j = entry->word0C; j >= 0; j--) {
            items[j] += entries[i];
        }
    }
    entries[count] += base;
    return -0xFF;
}

