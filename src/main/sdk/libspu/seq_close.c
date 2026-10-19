#include "common.h"

void _SsClose(s16 arg0) {
    s32 i;

    SpuVmSetSeqVol(arg0, 0, 0, 1);
    SpuVmSeqKeyOff(arg0);
    g_ss_open_seq_mask &= ~(1 << arg0);
    for (i = 0; i < g_ss_sep_max; i++) {
        g_ss_score_table[arg0][i].flags = 0;
        g_ss_score_table[arg0][i].next_sep = 0xFF;
        g_ss_score_table[arg0][i].open_sep_no = 0;
        g_ss_score_table[arg0][i].crescendo_step = 0;
        g_ss_score_table[arg0][i].crescendo_target = 0;
        g_ss_score_table[arg0][i].crescendo_ticks = 0;
        g_ss_score_table[arg0][i].crescendo_cnt = 0;
        g_ss_score_table[arg0][i].tempo_step = 0;
        g_ss_score_table[arg0][i].tempo_ticks = 0;
        g_ss_score_table[arg0][i].tempo_cnt = 0;
        g_ss_score_table[arg0][i].tempo_accum = 0;
        g_ss_score_table[arg0][i].tempo_diff = 0;
        g_ss_score_table[arg0][i].voll = 0x7F;
        g_ss_score_table[arg0][i].volr = 0x7F;
    }
}

void SsSeqClose(s16 arg0) {
    _SsClose(arg0);
}
