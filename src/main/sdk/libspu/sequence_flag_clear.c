#include "common.h"
/* The packed selector is passed as a word by this SDK caller. */
void _SsSndPause(s16 sequence,s16 channel)
{
    SequenceState *state = &g_ss_score_table[sequence][channel];
    SpuVmSeqKeyOff((channel<<8)|sequence);
    state->play_mode = 0;
    g_ss_score_table[sequence][channel].flags &= ~2;
}
