#include "common.h"
void Snd_SetPlayMode(s16 sequence,s16 channel,u8 mode,s16 repeat) {
    SequenceState *state = &g_ss_score_table[sequence][channel];
    state->flags &= ~0x200;
    g_ss_score_table[sequence][channel].flags &= ~4;
    state->loop_max = repeat;
    if (mode==1) {
        g_ss_score_table[sequence][channel].flags |= 1;
        state->loop_count = 0;
        state->play_mode = 1;
        SpuVmSetSeqVol((channel<<8)|sequence, state->voll, state->volr, 0);
    } else if (mode==0) {
        g_ss_score_table[sequence][channel].flags |= 2;
    }
}
