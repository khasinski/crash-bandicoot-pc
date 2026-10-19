#include "common.h"
#define CRASH_EXT_SDK_LIBSPU_BANK_SELECT
#include "sdk_externs.h"
#undef CRASH_EXT_SDK_LIBSPU_BANK_SELECT

s32 SpuVmVSetUp(s16 arg0, s16 arg1) {
    u8 *temp_a0, *master;

    if ((u16)arg0 >= 16 || g_midi_tracks[arg0] != 1 || arg1 >= g_vab_cur_prog_count) return -1;
    master = g_vab_master_vol_table[arg0];
    temp_a0 = g_vab_prog_tables[arg0];
    g_spu_vm_cur_vab = arg0;
    g_spu_vm_cur_prog = arg1;
    g_vab_cur_master_vol = master;
    g_vab_cur_prog_table = temp_a0;
    g_ns_vpage_pool = g_vab_tone_tables[arg0];
    g_spu_vm_cur_prog_tones = temp_a0[arg1 * 16 + 8];
    return 0;
}
