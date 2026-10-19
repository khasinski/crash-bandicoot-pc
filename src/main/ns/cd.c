#include "common.h"
#include "resource.h"

/* Call the CD routine with the inverted flag; report success only for a zero result. */

s32 Ns_ReadSync(s32 enabled)
{
    s32 result = CdReadSync(enabled == 0, 0);
    if (result < 0)
        return 0;
    return result == 0;
}

typedef struct CdResourceHash {
    u8 reserved_00[0x418];
    s32 word418;
    s32 word41C;
    u32 words420[1];
} CdResourceHash;

typedef struct CdResourceEntry {
    u8 reserved_00[0x14];
    s32 word14;
} CdResourceEntry;

typedef struct CdResourceSlot {
    u8 *header;
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
    CdResourceEntry *word24;
} CdResourceSlot;

s32 Ns_ReadSectorAsync(CdResourceSlot *slot, s32 count)
{
    u8 loc[16];
    s32 id;
    s32 sectors;
    u8 *buf;
    s32 pos;

    id = slot->id;
    if (slot->word0E != 0) {
        sectors = (count << 5) - slot->word0C;
        buf = slot->header + (slot->word0C << 11);
        pos = g_ns_nsf_base_sector + (((CdResourceHash *)g_ns_nsd_header)->words420[id >> 1] >> 6);
    } else {
        sectors = count << 5;
        pos = g_ns_nsf_base_sector + ((CdResourceHash *)g_ns_nsd_header)->word418 + ((id >> 1) << 5);
        buf = slot->header;
    }
    CdIntToPos(pos, loc);
    CdControlF(2, loc);
    if (CdRead(sectors, buf, 0x80) == 0) {
        return -0x10;
    }
    return -0xFF;
}

s32 Ns_PageOnComplete(CdResourceSlot *slot, s32 id)
{
    s32 max;

    if (id == -1) {
        return 1;
    }
    id >>= 1;
    max = slot->word24->word14 >> 1;
    if (id <= max && id > max - slot->word1C) {
        return 1;
    }
    return 0;
}

