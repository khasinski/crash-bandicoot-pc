#include "common.h"
void _SsSetPitchBend(s16 sequence,s16 channel) {
    SequenceState *state=g_ss_score_table[sequence]+channel;
    u8 *bytes=(u8 *)state;
    u8 *cursor=state->read_pos;
    u8 value;
    state->read_pos=cursor+1;
    value=*cursor;
    SpuVmPitchBend((channel<<8)|sequence,state->vab_id,bytes[0x2C+bytes[0x12]],value);
    state->step_counter=_SsReadDeltaValue(sequence,channel);
}
