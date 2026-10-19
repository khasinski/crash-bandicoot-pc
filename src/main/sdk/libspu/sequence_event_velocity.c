#include "common.h"
/* This SDK call passes promoted words, including the packed selector. */
void _SsContModulation(s16 sequence,s16 channel,u8 velocity) {
    SequenceState *state=g_ss_score_table[sequence]+channel;
    s32 index=state->channel;
    SpuVmSetVol((channel<<8)|sequence,state->vab_id,state->programs[index],velocity,state->panpot[index]);
    state->gains[index]=velocity;
    state->step_counter=_SsReadDeltaValue(sequence,channel);
}
