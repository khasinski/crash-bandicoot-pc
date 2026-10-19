#include "common.h"
void _SsSetProgramChange(s16 sequence,s16 channel)
{
    SequenceState *state=g_ss_score_table[sequence]+channel;
    u8 *cursor=state->read_pos;
    state->read_pos=cursor+1;
    state->vab_id=*cursor;
    state->step_counter=_SsReadDeltaValue(sequence,channel);
}
