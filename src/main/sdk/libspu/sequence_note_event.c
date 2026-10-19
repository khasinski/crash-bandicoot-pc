#include "common.h"
void _SsNoteOn(s16 sequence,s16 channel,u8 note,u8 velocity) {
    SequenceState *state=g_ss_score_table[sequence]+channel;
    s32 index=state->channel;
    u8 *slot=&state->open_sep_no+index;
    u8 parameter=slot[0x17];
    u16 volume=velocity;
    if (!((state->channel_mute>>index)&1)) {
        if (state->voll!=0) {
            if(velocity!=0) {
                SpuVmKeyOn((channel<<8)|sequence,state->vab_id,slot[0x2C],note,volume,parameter);
                state->velocity=volume;
            } else {
                SpuVmKeyOff((channel<<8)|sequence,state->vab_id,slot[0x2C],note);
            }
        }
    }
}
