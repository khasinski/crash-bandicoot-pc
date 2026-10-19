#include "common.h"
#include "resource.h"

/* Select 0x11, 0x22 or 0x33 for the observed input ranges. */
s32 Gfx_TgeoGetTexinfoArray(s32 arg0)
{
    if (arg0 < 0) {
        arg0 = 0x22;
    } else if (arg0 < 0x13) {
        arg0 = 0x11;
    } else if (arg0 < 0x24) {
        arg0 = 0x22;
    } else if (arg0 < 0x35) {
        arg0 = 0x33;
    }
    return arg0;
}

typedef struct {
    u32 words[5];
    struct {
        u8 reserved[3];
        u8 flags;
    } entries[1];
} RenderEntryData;

typedef struct {
    u8 reserved_00[0x10];
    RenderEntryData *data;
} RenderEntry;

s32 Gfx_TgeoLookupEntry(RenderEntry *entry)
{
    RenderEntryData *data = entry->data;
    u32 i;

    for (i = 0; i < data->words[6]; i++) {
        Ns_Lookup(&data->words[i + 8]);
    }
    return -0xFF;
}

s32 Gfx_TgeoOnLoad(RenderEntry *entry)
{
    RenderEntryData *data = entry->data;
    u32 i = 0;

    if (gp_pbak_mode == 0x19 || gp_pbak_mode == 0x38) {
        while (i < data->words[4]) {
            if (data->entries[i].flags & 0x80) {
                if (data->words[i + 6] & 1) {
                    data->words[i + 6] = Ns_Probe(data->words[i + 6]);
                }
                i += 3;
            } else {
                i += 1;
            }
        }
    } else {
        while (i < data->words[4]) {
            if (data->entries[i].flags & 0x80) {
                Ns_Open(&data->words[i + 6], 0, 1);
                i += 3;
            } else {
                i += 1;
            }
        }
    }
    return -0xFF;
}
