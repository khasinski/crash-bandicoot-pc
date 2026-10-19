#include "common.h"

s16 SpuVmGetSeqVol(s16 arg0, u16 *arg1, u16 *arg2) {
    union { s32 channel; u16 *packed_id; } phase;
    SequenceState *temp_v0;

    phase.channel = (arg0 & 0xFF00) >> 8;
    temp_v0 = &g_ss_score_table[arg0 & 255][phase.channel];
    phase.packed_id = (u16 *)&g_spu_vm_cur_seq_sep_id;
    *phase.packed_id = arg0;
    *arg1 = temp_v0->voll;
    *arg2 = temp_v0->volr;
    return (s16)*phase.packed_id;
}
