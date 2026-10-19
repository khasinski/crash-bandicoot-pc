#include "common.h"
void _SsContDataEntry(s16 sequence,s16 channel) {
    SequenceState *state=g_ss_score_table[sequence]+channel;
    SsUtReverbOff();
    SpuVmDamperOff();
    state->programs[state->channel]=state->channel;
    state->rpn_flag=0;
    state->nrpn_voice=0;
    state->gains[state->channel]=127;
    state->panpot[state->channel]=64;
    state->step_counter=_SsReadDeltaValue(sequence,channel);
}
