#include "common.h"
#include "audio_channels.h"

/* Wait for a pending fade, then request the selected sequence. Tagged
 * references contain an ID; resolved references point to a handle whose
 * second word is the ID. The fourth fade argument is retained as observed
 * at this call site, although the current callee consumes three words. */

void Midi_Update(s32 *reference) {
 s16 countdown;
 s32 key,ready,state;
 if(gp_card_chan!=0) {
  if(gp_card_chan==4 && (gp_midi_seq_num==0 || (countdown=gp_midi_seq_num-1,gp_midi_seq_num=countdown,countdown==0)))gp_card_chan=1;
  state=gp_card_chan;
  if(state==3) {
   key=*reference;
   if(key&1) {
    if(gp_midi_loaded_eid!=key)goto change;
    goto finish;
   }
   if(gp_midi_loaded_eid!=RAM_AT(s32, key)[1]) {
change:
    if(gp_midi_default_rvol==0 && gp_midi_vab_id_2!=-1 && gp_midi_vab_id==0) {
     SsSepSetDecrescendo(gp_midi_vol,g_midi_channel_states[0].channel,(s32)(gp_midi_seq_gain<<16)>>23,30);
     gp_midi_seq_num=30;gp_midi_vab_id=Ram_Addr(reference);gp_card_chan=4;return;
    }
   }
   state=gp_card_chan;
  }
finish:
  ready=state<3;
  if(ready && state>0)Midi_OpenAndPlay(reference);
 }
}
