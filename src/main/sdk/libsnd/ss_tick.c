#include "common.h"

void SsSeqCalledTbyT(void) {
    s32 var_s0;
    s32 var_s2;

    if (g_ss_callback_busy == 1) {
        return;
    }
    g_ss_callback_busy = 1;
    SpuVmFlush();
    for (var_s2 = 0; var_s2 < g_ss_seq_max; var_s2++) {
        if ((1 << var_s2) & g_ss_open_seq_mask) {
            for (var_s0 = 0; var_s0 < g_ss_sep_max; var_s0++) {
                if (g_ss_score_table[var_s2][var_s0].flags & 1) {
                    _SsSndPlay(var_s2, var_s0);
                    if (g_ss_score_table[var_s2][var_s0].flags & 0x10) {
                        _SsSndCrescendo(var_s2, var_s0);
                    }
                    if (g_ss_score_table[var_s2][var_s0].flags & 0x20) {
                        _SsSndDecrescendo(var_s2, var_s0);
                    }
                    if (g_ss_score_table[var_s2][var_s0].flags & 0x40) {
                        _SsSndSetRit(var_s2, var_s0);
                    }
                    if (g_ss_score_table[var_s2][var_s0].flags & 0x80) {
                        _SsSndSetRit(var_s2, var_s0);
                    }
                }
                if (g_ss_score_table[var_s2][var_s0].flags & 2) {
                    _SsSndPause(var_s2, var_s0);
                }
                if (g_ss_score_table[var_s2][var_s0].flags & 8) {
                    _SsSndReplay(var_s2, var_s0);
                }
                if (g_ss_score_table[var_s2][var_s0].flags & 4) {
                    _SsSndStop(var_s2, var_s0);
                    g_ss_score_table[var_s2][var_s0].flags = 0;
                }
            }
        }
    }
    g_ss_callback_busy = 0;
}

void _SsSndCrescendo(s16 arg0, s16 arg1) {
    u16 sp10;
    u16 sp12;
    SequenceState *temp_s0;

    temp_s0 = &g_ss_score_table[arg0][arg1];
    temp_s0->crescendo_cnt -= 1;
    if (temp_s0->tempo_step > 0) {
        if ((temp_s0->crescendo_cnt % temp_s0->tempo_step) == 0) {
            temp_s0->crescendo_target -= 1;
            if (temp_s0->crescendo_target >= 0) {
                SpuVmGetSeqVol((arg1 << 8) | arg0, &sp10, &sp12);
                if ((sp10 + 1) <= (sp10 + temp_s0->crescendo_target)) {
                    SpuVmSetSeqVol((arg1 << 8) | arg0, sp10 + 1, sp12 + 1, 1);
                }
            } else {
                SpuVmSetSeqVol((arg1 << 8) | arg0, 0x7F, 0x7F, 1);
                g_ss_score_table[arg0][arg1].flags &= ~0x10;
            }
            if ((temp_s0->crescendo_cnt == 0) || (temp_s0->crescendo_target == 0)) {
                g_ss_score_table[arg0][arg1].flags &= ~0x10;
            }
        }
    } else if (temp_s0->tempo_step < 0) {
        temp_s0->crescendo_target += temp_s0->tempo_step;
        if (temp_s0->crescendo_target >= 0) {
            SpuVmGetSeqVol((arg1 << 8) | arg0, &sp10, &sp12);
            if (((sp10 - temp_s0->tempo_step) >= 0x7F) && ((sp12 - temp_s0->tempo_step) >= 0x7F)) {
                SpuVmSetSeqVol((arg1 << 8) | arg0, 0x7F, 0x7F, 1);
            }
            if ((u32)((temp_s0->crescendo_ticks - temp_s0->crescendo_cnt) * -temp_s0->tempo_step) < (u32)temp_s0->crescendo_step) {
                SpuVmSetSeqVol((arg1 << 8) | arg0, sp10 - temp_s0->tempo_step, sp12 - temp_s0->tempo_step, 1);
            }
        } else {
            SpuVmSetSeqVol((arg1 << 8) | arg0, 0x7F, 0x7F, 1);
            g_ss_score_table[arg0][arg1].flags &= ~0x10;
        }
        if ((temp_s0->crescendo_cnt == 0) || (temp_s0->crescendo_target == 0)) {
            g_ss_score_table[arg0][arg1].flags &= ~0x10;
        }
    }
    SpuVmGetSeqVol((arg1 << 8) | arg0, &temp_s0->start_voll, &temp_s0->start_volr);
}

void _SsSndDecrescendo(s16 arg0, s16 arg1) {
    u16 sp10;
    u16 sp12;
    SequenceState *temp_s0;

    temp_s0 = &g_ss_score_table[arg0][arg1];
    temp_s0->crescendo_cnt -= 1;
    if (temp_s0->tempo_step > 0) {
        if ((temp_s0->crescendo_cnt % temp_s0->tempo_step) == 0) {
            temp_s0->crescendo_target -= 1;
            if (temp_s0->crescendo_target > 0) {
                SpuVmGetSeqVol((arg1 << 8) | arg0, &sp10, &sp12);
                if (((sp10 - temp_s0->crescendo_target) <= 0) || ((sp12 - temp_s0->crescendo_target) <= 0) || (sp10 == 1)) {
                    SpuVmSetSeqVol((arg1 << 8) | arg0, 1, 1, 1);
                } else {
                    SpuVmSetSeqVol((arg1 << 8) | arg0, sp10 - 1, sp12 - 1, 1);
                }
            } else {
                g_ss_score_table[arg0][arg1].flags &= ~0x20;
            }
            if ((temp_s0->crescendo_cnt == 0) || (temp_s0->crescendo_target == 0)) {
                g_ss_score_table[arg0][arg1].flags &= ~0x20;
            }
        }
    } else {
        temp_s0->crescendo_target += temp_s0->tempo_step;
        if (temp_s0->crescendo_target > 0) {
            SpuVmGetSeqVol((arg1 << 8) | arg0, &sp10, &sp12);
            if (((u32)temp_s0->crescendo_step >= (u32)((temp_s0->crescendo_ticks - temp_s0->crescendo_cnt) * -temp_s0->tempo_step)) && (-temp_s0->tempo_step < sp10)) {
                SpuVmSetSeqVol((arg1 << 8) | arg0, sp10 + temp_s0->tempo_step, sp12 + temp_s0->tempo_step, 1);
            } else {
                SpuVmSetSeqVol((arg1 << 8) | arg0, 1, 1, 1);
            }
        } else {
            g_ss_score_table[arg0][arg1].flags &= ~0x20;
        }
        if ((temp_s0->crescendo_cnt == 0) || (temp_s0->crescendo_target == 0)) {
            g_ss_score_table[arg0][arg1].flags &= ~0x20;
        }
    }
    SpuVmGetSeqVol((arg1 << 8) | arg0, &temp_s0->start_voll, &temp_s0->start_volr);
}

