#include "common.h"

void _SsSndStop(s16 arg0, s16 arg1) {
    s32 var_t0;
    SequenceState *temp_s0;

    temp_s0 = &g_ss_score_table[arg0][arg1];
    g_ss_score_table[arg0][arg1].flags &= ~1;
    g_ss_score_table[arg0][arg1].flags &= ~2;
    g_ss_score_table[arg0][arg1].flags &= ~8;
    g_ss_score_table[arg0][arg1].flags |= 4;
    SpuVmSeqKeyOff((arg1 << 8) | arg0);
    temp_s0->play_mode = 0;
    temp_s0->delta_value = 0;
    temp_s0->rpn2 = 0;
    temp_s0->rpn_flag = 0;
    temp_s0->nrpn_voice = 0;
    temp_s0->nrpn_mode = 0;
    temp_s0->nrpn2 = 0;
    temp_s0->rpn1 = 0;
    temp_s0->nrpn_count = 0;
    temp_s0->channel = 0;
    temp_s0->loop_count = 0;
    temp_s0->rpn2 = 0;
    temp_s0->data_entry = 0;
    temp_s0->nrpn1 = 0;
    temp_s0->last_status = 0;
    temp_s0->step_counter = temp_s0->tempo_base;
    temp_s0->cur_bpm = temp_s0->init_bpm;
    temp_s0->tick_rem = temp_s0->tick_init;
    temp_s0->read_pos = temp_s0->start_pos;
    temp_s0->loop_pos = temp_s0->start_pos;
    for (var_t0 = 0; var_t0 < 0x10; var_t0++) {
        temp_s0->programs[var_t0] = var_t0;
        temp_s0->panpot[var_t0] = 0x40;
        temp_s0->gains[var_t0] = 0x7F;
    }
    temp_s0->start_voll = 0x7F;
    temp_s0->start_volr = 0x7F;
}
