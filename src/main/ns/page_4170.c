#include "common.h"
#include "ns.h"
#include "zone.h"

typedef struct NsDirectoryExt {
    s32 bucket_offsets[0x100];
    s32 page_count;
    s32 entry_count;
    u32 level_eid;
    s32 word40C;
    s32 word410;
    s32 word414;
    s32 word418;
    s32 word41C;
    u32 words420[1];
} NsDirectoryExt;

typedef struct PageSlot {
    RAM_REF(u8) header;
    u16 state;
    s16 word06;
    s16 word08;
    s16 references;
    s16 word0C;
    s16 word0E;
    u32 word10;
    s32 id;
    s32 word18;
    s32 word1C;
    s32 word20;
    RAM_REF(struct PageSlot) slot;
    u32 word28;
} PageSlot;

void Ns_PageDecompress(u8 *src, u8 *dst)
{
    u32 b0, b1, b2, b3;
    u32 b4, b5, b6, b7;
    u32 b8, b9, b10, b11;
    u32 magic;
    s32 comp_len;
    s32 skip_len;
    s32 rem;
    s32 len;
    u8 *back;

    b0 = src[0];
    b1 = src[1];
    b2 = src[2];
    b3 = src[3];
    src += 4;
    b4 = src[0];
    b5 = src[1];
    b6 = src[2];
    b7 = src[3];
    src += 4;
    b8 = src[0];
    b9 = src[1];
    b10 = src[2];
    b11 = src[3];
    src += 4;

    comp_len = b4 + (b5 << 8) + (b6 << 16) + (b7 << 24);
    magic = b0 + (b1 << 8) + (b2 << 16) + (b3 << 24);
    skip_len = b8 + (b9 << 8) + (b10 << 16) + (b11 << 24);

    if ((magic & 0xFFFF) == 0x1234) {
        if (src != dst) {
            src -= 12;
            for (len = 0; len <= 0xFFFF; len++) {
                *dst++ = *src++;
            }
        }
        return;
    }

    rem = 0x10000 - comp_len;
    if (comp_len != 0) {
        do {
            len = *src++;
            if (len & 0x80) {
                len = (len << 8) + *src++;
                back = dst - ((len >> 3) & 0xFFF);
                len &= 7;
                if (len == 7) {
                    len = 0x40;
                } else {
                    len += 3;
                }
                comp_len -= len;
                while (--len != -1) {
                    *dst++ = *back++;
                }
            } else {
                comp_len -= len;
                while (--len != -1) {
                    *dst++ = *src++;
                }
            }
        } while (comp_len != 0);
    }
    src += skip_len;
    while (--rem != -1) {
        *dst++ = *src++;
    }
}

static __inline__ s32 ns_page_resolve(ResourceHeader *page)
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

static __inline__ s32 ns_cd_check(s32 enabled)
{
    s32 result = CdReadSync(enabled == 0, 0);
    if (result < 0)
        return 0;
    return result == 0;
}

static __inline__ s32 ns_cd_read(PageSlot *slot, s32 count)
{
    CdlLOC loc;
    s32 id;
    s32 sectors;
    u8 *buf;
    s32 pos;
    NsDirectoryExt *directory = RAM_AT(NsDirectoryExt, g_ns.directory);

    id = slot->id;
    if (slot->word0E != 0) {
        sectors = (count << 5) - slot->word0C;
        buf = RAM_AT(u8, slot->header) + (slot->word0C << 11);
        pos = g_ns.nsf_sector + (directory->words420[id >> 1] >> 6);
    } else {
        sectors = count << 5;
        pos = g_ns.nsf_sector + directory->word418 + ((id >> 1) << 5);
        buf = RAM_AT(u8, slot->header);
    }
    CdIntToPos(pos, &loc);
    CdControlF(2, (u_char *)&loc);
    if (CdRead(sectors, (u_long *)buf, 0x80) == 0) {
        return -0x10;
    }
    return -0xFF;
}

PageSlot *Ns_PageTranslate(PageSlot *slot)
{
    s32 state = (s16)slot->state;
    s32 count = slot->word1C;
    s32 offset = 0;
    s32 compressed;
    s32 i;
    s32 next_offset;
    PageSlot *prev;
    NsDirectoryExt *directory = RAM_AT(NsDirectoryExt, g_ns.directory);

    if (state == 3 || state == 0xD) {
        if (slot->word0E != 0) {
            Ns_PageDecompress(RAM_AT(u8, slot->header) + (slot->word0C << 11), RAM_AT(u8, slot->header));
        }
        slot->state = 4;
        ns_page_resolve(RAM_AT(ResourceHeader, slot->header));
        return NULL;
    }

    compressed = (slot->id >> 1) < directory->word41C && g_ns.wordA98 != 0;
    for (i = 0; i < count; i++) {
        if (state == 2) {
            slot->word0E = compressed;
            if (compressed != 0) {
                next_offset = offset + 0x20;
                offset = next_offset - (directory->words420[slot->id >> 1] & 0x3F);
                slot->word0C = offset;
            }
            if ((s16)slot->state == state) {
                slot->state = 3;
            } else {
                slot->state = 0xD;
                if (compressed != 0) {
                    slot->word10 = slot->header + (((directory->words420[slot->id >> 1] & 0x3F) + slot->word0C) << 11);
                } else {
                    slot->word10 = slot[1].header;
                }
                *RAM_AT(u16, slot->word10) = 0x8765;
            }
        }
        prev = slot;
        slot--;
    }
    return prev;
}

void Ns_PageUpdate(PageSlot *slot, s32 id)
{
    PageSlot *next_slot;
    s32 shifted_id;
    s32 max;
    s32 in_range;
    s32 arg1;
    u16 kind;
    s32 idx;
    ResourceSlot *spu_slot;
    ResourceHeader *header;
    ResourceEntry *entry;

loop:
    switch ((s16)slot->state) {
    case 13:
        if (id == -1) {
            if (*RAM_AT(u16, slot->word10) == 0x8765) {
                break;
            }
        } else if (!ns_cd_check(1)) {
            break;
        }
        Ns_PageTranslate(slot);
        if (id != -1 && Ram_Addr(slot) != slot->slot) {
            Ns_PageUpdate(slot + 1, id);
        }
        goto case_4;
    case 2:
        if (g_ns.wordA84 != 0) {
            break;
        }
        if (id != -1) {
            shifted_id = id >> 1;
            max = RAM_AT(PageSlot, slot->slot)->id >> 1;
            in_range = shifted_id <= max && shifted_id > max - slot->word1C;
            if (!in_range) {
                break;
            }
        }
        g_ns.wordA90 = slot->id;
        if (gp_pause_B_pressed == 0) {
            goto abort_cd;
        }
        if (CdSync(1, 0) != 2 && id == -1) {
            break;
        }
        next_slot = Ns_PageTranslate(slot);
        g_ns.wordA84 = (s32)Ram_Addr(slot);
        ns_cd_read(next_slot, slot->word1C);
        slot = next_slot;
        goto loop;
    case 3:
        if (gp_pause_B_pressed != 0 && (u32)g_ns.wordA84 == Ram_Addr(slot)) {
            if (id == -1) {
                if (ns_cd_check(0)) {
                    goto finish_cd;
                }
                if (id == -1) {
                    break;
                }
            }
            if (!ns_cd_check(1)) {
                break;
            }
        finish_cd:
            Ns_PageTranslate(slot);
            g_ns.wordA84 = 0;
        } else {
        abort_cd:
            Ns_PageDelete((ResourceSlot *)slot);
            g_ns.wordA84 = 0;
            g_ns.pool.active_count--;
            break;
        }
    case 4:
    case_4:
        if ((u16)(RAM_AT(ResourceHeader, slot->header)->kind - 3) >= 2) {
            Ns_PageUpdateEntries(slot->word08);
            g_ns.pool.active_count--;
            break;
        }
        slot->state = 10;
    case 10:
        if (g_ns.wordA8C != 0) {
            break;
        }
        if (id != -1 && id != slot->id) {
            break;
        }
        arg1 = -1;
        kind = RAM_AT(ResourceHeader, slot->header)->kind;
        if (kind == 4) {
            entry = RAM_AT(ResourceEntry, RAM_AT(ResourceHeader, slot->header)->entries[0]);
            arg1 = *RAM_AT(s32, entry->data) + g_ns.audio_page_count;
        }
        idx = Ns_AudioPageAllocate(kind, arg1);
        if (idx == -12) {
            Ns_PageDelete((ResourceSlot *)slot);
            g_ns.pool.active_count--;
            break;
        }
        spu_slot = &g_spu_slots[idx];
        slot->state = 11;
        slot->slot = Ram_Addr(spu_slot);
        g_ns.wordA8C = (s32)Ram_Addr(slot);
        spu_slot->state = 11;
        spu_slot->id = slot->id;
        SpuSetTransferStartAddr(spu_slot->base);
        header = RAM_AT(ResourceHeader, slot->header);
        if (header->kind == 3) {
            SpuWrite((u_char *)header, 0x10000);
        } else if (header->kind == 4) {
            entry = RAM_AT(ResourceEntry, header->entries[0]);
            /* Item 1 is the sample data; item 0 starts with its size. */
            SpuWrite(RAM_AT(u_char, (&entry->data)[1]), RAM_AT(s32, entry->data)[1]);
        }
    case 11:
        if ((u32)g_ns.wordA8C == Ram_Addr(slot)) {
            if (SpuIsTransferCompleted(id == slot->id)) {
                Ns_PageUpdateEntries(slot->word08);
                g_ns.wordA8C = 0;
                g_ns.pool.active_count--;
            }
        }
        break;
    case 31:
        slot->state = 1;
        break;
    }
}

s32 Ns_PageAllocate(s32 *count)
{
    s32 best_start = -1;
    s32 best_len;
    u32 best_cost;
    s32 i;
    s32 j;
    u32 cost;
    ResourceSlot *slot;
    ResourceSlot *phys;
    s16 state;

    for (i = 0; i < g_ns.physical_count; i++) {
        phys = g_ns.physical;
        cost = 0;
        for (j = 0; j < (*count < g_ns.physical_count - i ? *count : g_ns.physical_count - i); j++) {
            slot = &phys[i + j];
            state = slot->state;
            if (state != 1) {
                if ((u16)(state - 20) >= 2) {
                    break;
                }
                if (slot->references > 0) {
                    break;
                }
                cost += slot->word28 + 1;
            }
            if (best_start == -1 || best_len < j + 1 || (best_len == j + 1 && cost < best_cost)) {
                best_start = i;
                best_len = j + 1;
                best_cost = cost;
            }
        }
    }
    if (best_start != -1) {
        for (j = best_start; j < best_start + best_len; j++) {
            Ns_PageFree(&g_ns.physical[j]);
        }
        *count = best_len;
        return best_start;
    }
    *count = 0;
    return -1;
}
