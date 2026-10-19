#include "common.h"
/* This SDK call passes promoted words, including the packed selector. */
void _SsContPortaTime(s16 sequence,s16 channel,u8 value) {
    SequenceState *state=g_ss_score_table[sequence]+channel;
    s32 index=state->channel;
    u8 *slot=&state->open_sep_no+index;
    SpuVmSetVol((channel<<8)|sequence,state->vab_id,slot[0x2C],state->gains[index],value);
    slot[0x17]=value;
    state->step_counter=_SsReadDeltaValue(sequence,channel);
}
