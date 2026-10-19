#include "common.h"

typedef struct {
    s32 form;
    s32 ver;
    s32 id;
    u32 fsize;
    u16 reserved0;
    u16 ps;
    u16 ts;
    u8 vs;
    u8 reserved;
    u8 mvol;
    u8 mpan;
    u8 attr1;
    u8 attr2;
    u32 reserved1;
} VabHdr;

typedef struct {
    u8 tones;
    u8 mvol;
    u8 prior;
    u8 mode;
    u8 mpan;
    s8 reserved0;
    s16 attr;
    u32 reserved1;
    u16 reserved2;
    u16 reserved3;
} ProgAtr;

#define CRASH_EXT_SDK_LIBSPU_BANK_TRANSFER
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_BANK_TRANSFER

s16 SsVabOpenHeadWithMode(u8 *addr, s16 vabid, s16 mode, u32 sbaddr) {
    u32 vag_len[256];
    s32 i;
    s16 var_s2;
    VabHdr *vab_hdr;
    ProgAtr *prog_ptr;
    u8 *ptr;
    u32 var_s0;
    u8 num_vags;
    u32 var_a3;

    var_s2 = 0x10;
    if (KeyOnCheck() == 1) {
        return -1;
    }
    SsSetMono(1);
    if (vabid < 0x10) {
        if (vabid == -1) {
            for (i = 0; i < 0x10; i++) {
                if (g_midi_tracks[i] == 0) {
                    goto found;
                }
            }
            goto check_s2;
        found:
            g_midi_tracks[i] = 1;
            var_s2 = i;
            g_vab_open_count += 1;
        } else {
            if (g_midi_tracks[vabid] == 0) {
                g_midi_tracks[vabid] = 1;
                var_s2 = vabid;
                g_vab_open_count += 1;
            }
        }
    } else {
        SsSetMono(0);
        return -1;
    }
check_s2:
    if (var_s2 >= 0x10) {
        SsSetMono(0);
        return -1;
    }
    ptr = addr;
    g_vab_master_vol_table[var_s2] = (VabHdr *)ptr;
    ptr += 0x20;
    vab_hdr = g_vab_master_vol_table[var_s2];
    if (((u32)vab_hdr->form >> 8) != 0x564142) {
        g_midi_tracks[var_s2] = 0;
        SsSetMono(0);
        g_vab_open_count -= 1;
        return -1;
    }
    if ((vab_hdr->form & 0xFF) == 0x70) {
        if (vab_hdr->ver < 5) {
            g_vab_cur_prog_count = 0x40;
        } else {
            g_vab_cur_prog_count = 0x80;
        }
    } else {
        g_vab_cur_prog_count = 0x40;
    }
    if (vab_hdr->ps > g_vab_cur_prog_count) {
        g_midi_tracks[var_s2] = 0;
        SsSetMono(0);
        g_vab_open_count -= 1;
        return -1;
    }
    g_vab_prog_tables[var_s2] = (ProgAtr *)ptr;
    prog_ptr = (ProgAtr *)ptr;
    ptr += g_vab_cur_prog_count * 0x10;
    var_s0 = 0;
    for (i = 0; i < g_vab_cur_prog_count; i++) {
        prog_ptr[i].reserved1 = var_s0;
        if (prog_ptr[i].tones != 0) {
            var_s0 += 1;
            var_a3 = i;
        }
    }
    g_vab_tone_tables[var_s2] = ptr;
    var_s0 = 0;
    ptr += vab_hdr->ps << 9;
    num_vags = vab_hdr->vs;
    for (i = 0; i < 256; i++) {
        if (i <= num_vags) {
            u16 raw_len = *(u16 *)ptr;
            if (vab_hdr->ver >= 5) {
                vag_len[i] = raw_len << 3;
            } else {
                vag_len[i] = raw_len << 2;
            }
            var_s0 += vag_len[i];
        }
        ptr += 2;
    }
    g_vab_vh_ptrs[var_s2] = ptr;
    if (mode == 0) {
        var_a3 = SpuMalloc(var_s0);
        if (var_a3 == -1) {
            g_midi_tracks[var_s2] = 0;
            SsSetMono(0);
            g_vab_open_count -= 1;
            return -1;
        }
    } else {
        var_a3 = sbaddr;
    }
    if ((u32)(var_a3 + var_s0) > 0x80000U) {
        g_midi_tracks[var_s2] = 0;
        SsSetMono(0);
        g_vab_open_count -= 1;
        return -1;
    }
    g_vab_spu_addrs[var_s2] = var_a3;
    var_s0 = 0;
    for (i = 0; i <= num_vags; i++) {
        var_s0 += vag_len[i];
        if ((i & 1) == 0) {
            prog_ptr[i / 2].reserved2 = (var_a3 + var_s0) >> 3;
        } else {
            prog_ptr[i / 2].reserved3 = (var_a3 + var_s0) >> 3;
        }
    }
    g_vab_body_sizes[var_s2] = var_s0;
    g_midi_tracks[var_s2] = 2;
    return var_s2;
}
