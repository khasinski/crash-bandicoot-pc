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
        item = (s32 *)((page->count << 2) + (s32)page);
    common:
        size = item[4];
        if (size < 0) {
            return size - (s32)page;
        }
        return size;
    case 0x100:
        item = (s32 *)((entry->word0C << 2) + (s32)entry);
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

void *Ns_ReadSectors(void *arg0, void *buffer, s32 read_full)
{
    u8 result[8];
    u8 position[4];
    NsFileMap *map;

    if (gp_pause_B_pressed) {
        if (read_full == 1) {
            map = (NsFileMap *)arg0;
            if (buffer == NULL) {
                buffer = LIBC_Malloc(((map->directory_size & 0x1FFFF) + 0x7FF) / 0x800 * 0x800);
                if (buffer == NULL) {
                    return (void *)-15;
                }
            }
            CdIntToPos(map->directory_sector & 0x7FFFFF, position);
            CdControl(2, position, result);
            if (!CdRead(((map->directory_size & 0x1FFFF) + 0x7FF) / 0x800, buffer, 0x80)
                || CdReadSync(0, 0)) {
                return (void *)-16;
            }
            buffer = LIBC_Realloc(buffer, map->directory_size & 0x1FFFF);
        }
        return buffer;
    }
}

s32 Ns_FileOpen(NsState *ns, s32 type)
{
    s32 i;
    s32 count = 0;

    for (i = 0; i < ns->directory->entry_count; i++) {
        if (Ns_EIDType(&ns->directory->entries[i]) == type) {
            count++;
        }
    }
    return count;
}

s32 Ns_PageTranslateOffsets(ResourceHeader *page)
{
    s32 count = page->count;
    ResourceEntry **entries = page->entries;
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
        entry = (ResourceEntry *)((u32)page + (u32)entries[i]);
        entries[i] = entry;
        items = (u32 *)&entry->data;
        for (j = entry->word0C; j >= 0; j--) {
            items[j] = (u32)entry + items[j];
        }
    }
    entries[count] = (ResourceEntry *)((u32)page + (u32)entries[count]);
    return -0xFF;
}

