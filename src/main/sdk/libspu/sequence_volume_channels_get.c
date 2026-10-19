#include "common.h"
s32 SpuVmGetSeqLVol(s32 selector) {
    s32 channel=(selector&0xFF00)>>8;
    SequenceState *state=&g_ss_score_table[selector&0xFF][channel];
    g_spu_vm_cur_seq_sep_id=selector;
    return *(s16 *)&state->voll;
}
s32 SpuVmGetSeqRVol(s32 selector) {
    s32 channel=(selector&0xFF00)>>8;
    SequenceState *state=&g_ss_score_table[selector&0xFF][channel];
    g_spu_vm_cur_seq_sep_id=selector;
    return *(s16 *)&state->volr;
}
