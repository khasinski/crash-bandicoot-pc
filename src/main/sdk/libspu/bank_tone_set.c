#include "common.h"
                          /* extern */

s32 SsUtSetVagAtr(s16 bank, s16 program, s32 tone, const ToneAttributes *result) {
    s32 temp_v1;
    ToneAttributes *temp_v1_2;

    if (g_midi_tracks[bank] == 1) {
        SpuVmVSetUp(bank, program);
        temp_v1 = (s32) (((u32)tone + (g_spu_vm_cur_prog_tones * 0x10)) << 0x10) >> 0xB;
        ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->prior = result->prior;
        ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->mode = result->mode;
        ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->vol = result->vol;
        ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->pan = result->pan;
        ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->center = result->center;
        ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->shift = result->shift;
        ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->max = result->max;
        ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->min = result->min;
        ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->vibW = result->vibW;
        ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->vibT = result->vibT;
        ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->porW = result->porW;
        ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->porT = result->porT;
        ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->pbmin = result->pbmin;
        ((ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool))->pbmax = result->pbmax;
        temp_v1_2 = (ToneAttributes *)(temp_v1 + (u32)g_ns_vpage_pool);
        temp_v1_2->adsr1 = result->adsr1;
        temp_v1_2->adsr2 = result->adsr2;
        temp_v1_2->prog = result->prog;
        temp_v1_2->vag = result->vag;
        return 0;
    }
    return -1;
}
