#include "common.h"
#include "ns.h"

/* Use the CD (disc map) rather than host files. Only this unit addresses the
 * flag through $gp, so it owns the .sdata word. */
s32 gp_pause_B_pressed = 1;

     /* "Inited and Allocated %d pages\n" */

/* The entry search of Ns_ProbeSafe, expanded inline at both uses. */
static __inline__ ResourceBucket *ns_lookup(u32 id)
{
    ResourceBucket *cursor = g_ns.buckets[(id >> 15) & 255];

    while (cursor->id != id) {
        cursor++;
        if (((s32)((u8 *)cursor - (u8 *)g_ns.entries) >> 3)
            >= g_ns.directory->entry_count) {
            return (ResourceBucket *)-10;
        }
    }
    return cursor;
}

/* Read a level's directory file through the disc map, in whole sectors. */
static __inline__ NsDirectory *ns_read_directory(s32 level)
{
    u8 result[8];
    u8 position[4];
    NsFileMap *map;
    NsDirectory *buffer;

    map = &g_file_map[level];
    buffer = LIBC_Malloc(((map->directory_size & 0x1FFFF) + 0x7FF) / 0x800 * 0x800);
    if (buffer == NULL) {
        return (NsDirectory *)-15;
    }
    CdIntToPos(map->directory_sector & 0x7FFFFF, position);
    CdControl(2, position, result);
    if (!CdRead(((map->directory_size & 0x1FFFF) + 0x7FF) / 0x800, buffer, 0x80)
        || CdReadSync(0, 0)) {
        return (NsDirectory *)-16;
    }
    return LIBC_Realloc(buffer, map->directory_size & 0x1FFFF);
}

/* Start paging for a level: read its directory, size the page map, allocate
 * physical pages and assign the VRAM texture pages and SPU audio pages. */
s32 Ns_Init(NsState *ns, s32 level)
{
    char filename[0x100];
    NsDirectory *directory;
    ResourceSlot *slot;
    ResourceBucket *entry;
    s32 i, j, column, row, page, clut, word, type;

    if (ns->initialized != 0) {
        return -0xFF;
    }
    gp_pbak_frame_idx = -1;
    gp_pbak_mode = level;
    ns->initialized = 1;
    ns->wordA84 = 0;
    ns->pool.active_count = 0;
    ns->wordA8C = 0;
    ns->level = level;
    ns->wordA90 = 0;
    ns->wordA98 = 1;
    BIOS_StrCpy(filename, (char *)Ns_GetFilename2(0, level, 0, 0));

    /* Without the CD the host-file read is compiled out; retail stores the
     * uninitialized local. */
    if (gp_pause_B_pressed) {
        ns->directory = ns_read_directory(level);
    } else {
        ns->directory = directory;
    }

    ns->level_eid = &ns->directory->level_eid;
    ns->buckets = (ResourceBucket **)ns->directory;
    ns->entries = ns->directory->entries;
    ns->level_header = (u8 *)&ns->directory->entries[ns->directory->entry_count];
    Title_Loading(level, ns->directory->entry_count * 8 + 0x730 + (u8 *)ns->directory, ns->directory);
    /* Drop the loading-screen image that follows the level header. */
    ns->directory = LIBC_Realloc(ns->directory, ns->directory->entry_count * 8 + 0x730);
    for (i = 0; i < 0x100; i++) {
        ns->buckets[i] = &ns->entries[((s32 *)ns->buckets)[i]];
    }

    BIOS_StrCpy(filename, (char *)Ns_GetFilename2(1, level, 0, 0));
    if (gp_pause_B_pressed) {
        g_ns.nsf_sector = g_file_map[level].page_sector & 0xFFFFFF;
    }
    ns->page_map = LIBC_Malloc(ns->directory->page_count * 4);
    for (i = 0; i < ns->directory->page_count; i++) {
        ns->page_map[i] = (ResourceSlot *)-18;
    }
    for (i = 0; i < 21; i++) {
        if (g_ns_bootstrap_ldat[i].init) {
            g_ns_bootstrap_ldat[i].init();
        }
    }

    /* Up to 22 physical 64 KB pages, fewer if memory is short. */
    ns->physical_count = 22;
    i = 22;
    do {
        if ((ns->physical[0].header = LIBC_Malloc(i << 16)) != NULL) {
            break;
        }
    } while (--i >= 0);
    ns->physical_count = i;
    for (i = 0; i < ns->physical_count; i++) {
        ns->physical[i].state = 1;
        ns->physical[i].word06 = 1;
        ns->physical[i].header = (ResourceHeader *)((u32)ns->physical[0].header + (i << 16));
        ns->physical[i].word08 = i;
        ns->physical[i].references = 0;
    }
    ns->pool.count = 38;
    for (i = 0; i < ns->pool.count; i++) {
        ns->pool.slots[i].state = 1;
        ns->pool.slots[i].word06 = 0;
        ns->pool.slots[i].header = NULL;
        ns->pool.slots[i].references = 0;
    }
    BIOS_Printf(g_str_init_inited_and_allocated_d, ns->physical_count);

    /* Texture pages: VRAM as a 4x4 grid of 256x128 blocks. Pages 0-3 hold the
     * display buffer and 4-7 the draw buffer; a page still loaded from the
     * previous level is kept if its entry exists in this level too. */
    for (i = 0; i < 16; i++) {
        g_slst_buf0[i].word06 = 2;
        column = i % 4;
        row = i / 4;
        page = column << 2;
        if (i >= 8) {
            page |= 0x10;
        }
        clut = (column << 8) | (row << 17);
        if (row & 1) {
            word = clut | 0x80;
            g_slst_buf0[i].base = (u8 *)(0x80000002 | (page | word));
        } else {
            g_slst_buf0[i].base = (u8 *)(0x80000002 | (page | clut));
        }
        if ((i >= 0 && i < 4) || (i >= 4 && i < 8)) {
            g_slst_buf0[i].state = 30;
        } else {
            slot = &g_slst_buf0[i];
            if (slot->state == 20 || slot->state == 21) {
                entry = ns_lookup(g_slst_buf0[i].word18);
                if ((s32)entry < -0xFE) {
                    g_slst_buf0[i].state = 20;
                    g_slst_buf0[i].id = entry->entry;
                    g_slst_buf0[i].mapping = (u32 *)entry;
                    g_ns.page_map[g_slst_buf0[i].id >> 1] = &g_slst_buf0[i];
                    entry->entry = (u32)g_slst_buf0[i].base;
                } else {
                    g_slst_buf0[i].state = 21;
                }
            } else {
                g_slst_buf0[i].state = 1;
            }
        }
    }

    /* Audio pages: eight SPU pages, minus one per wavebank entry (type 14) in
     * the level. The level number is no longer needed and counts them. */
    type = 14;
    level = 0;
    for (j = 0; j < ns->directory->entry_count; j++) {
        if (Ns_EIDType(&ns->directory->entries[j]) == type) {
            level++;
        }
    }
    g_ns.audio_page_count = 8 - level;
    if (g_ns.audio_page_count < 2) {
        g_ns.audio_page_count = 2;
    }
    switch (gp_pbak_mode) {
    case 3: case 0x22: case 0x29: case 0x2B: case 0x2E: case 0x38:
        break;
    default:
        if (g_ns.audio_page_count >= 5) {
            g_ns.audio_page_count = 4;
        }
        break;
    }
    for (i = 0; i < 8; i++) {
        g_spu_slots[i].word08 = i;
        if (i < g_ns.audio_page_count) {
            g_spu_slots[i].state = 1;
            g_spu_slots[i].word06 = 3;
            g_spu_slots[i].base = (u8 *)(0x2000 + (i << 16));
        } else {
            g_spu_slots[i].state = 1;
            g_spu_slots[i].word06 = 4;
            g_spu_slots[i].base = (u8 *)(0x2000 + (g_ns.audio_page_count << 16)
                                        + 0xFF90 * (i - g_ns.audio_page_count));
        }
    }
    /* Keep instrument pages whose entries are still present. */
    for (i = 0; i < 8; i++) {
        if (g_ns_recent_page_eids[i] != 0x6396347F && g_spu_slots[i].word06 == 4) {
            entry = ns_lookup(g_ns_recent_page_eids[i]);
            if ((s32)entry >= -0xFE) {
                g_ns_recent_page_eids[i] = 0x6396347F;
            } else {
                g_ns.page_map[(s32)entry->entry >> 1] = &g_spu_slots[i];
                g_spu_slots[i].state = 20;
                g_spu_slots[i].id = entry->entry;
                g_spu_slots[i].word18 = g_ns_recent_page_eids[i];
                entry->entry = 0x80000002;
            }
        }
    }

    for (i = 0; i < 21; i++) {
        if (g_ns_bootstrap_ldat[i].init_pages) {
            g_ns_bootstrap_ldat[i].init_pages();
        }
    }
    return -0xFF;
}

/* Shut down subsystem tables, release all physical pages and free paging buffers. */
s32 Ns_Kill(NsState *ns)
{
    s32 i;

    if (ns->initialized != 1) {
        return -0xFF;
    }
    for (i = 0; i < 21; i++) {
        if (g_ns_bootstrap_ldat[i].kill) {
            g_ns_bootstrap_ldat[i].kill();
        }
    }
    for (i = 0; i < ns->physical_count; i++) {
        Ns_KillPage(ns, i);
    }
    LIBC_Free(ns->physical[0].header);
    LIBC_Free(ns->page_map);
    LIBC_Free(ns->directory);
    ns->initialized = 0;
    return -0xFF;
}
