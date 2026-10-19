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

/* Entry data words; the top byte of word i + 5 flags a three-word item. */
typedef struct RenderEntry {
    u8 reserved_00[0x10];
    RAM_REF(u32) data;
} RenderEntry;

s32 Gfx_TgeoLookupEntry(RenderEntry *entry)
{
    u32 *words = RAM_AT(u32, entry->data);
    u32 i;

    for (i = 0; i < words[6]; i++) {
        Ns_Lookup(&words[i + 8]);
    }
    return -0xFF;
}

s32 Gfx_TgeoOnLoad(RenderEntry *entry)
{
    u32 *words = RAM_AT(u32, entry->data);
    u32 i = 0;

    if (gp_pbak_mode == 0x19 || gp_pbak_mode == 0x38) {
        while (i < words[4]) {
            if (((u8 *)&words[i + 5])[3] & 0x80) {
                if (words[i + 6] & 1) {
                    words[i + 6] = Ram_Addr(Ns_Probe(words[i + 6]));
                }
                i += 3;
            } else {
                i += 1;
            }
        }
    } else {
        while (i < words[4]) {
            if (((u8 *)&words[i + 5])[3] & 0x80) {
                Ns_Open(&words[i + 6], 0, 1);
                i += 3;
            } else {
                i += 1;
            }
        }
    }
    return -0xFF;
}
