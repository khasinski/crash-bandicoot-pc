#include "common.h"
#include "audio_channels.h"

                               /* extern */
                          /* extern */
                /* extern */
                          /* extern */
                          /* extern */
                /* extern */
                               /* extern */

/* Export the real small-common channel identifier. */
extern s16 gp_midi_vab_id_2;

void Midi_Close(s16 arg0, s32 arg1) {
    if (arg0 != -1) {
        SsSepClose(arg0);
    }
    if ((arg1 != 0) && (gp_midi_vab_id_2 != -1)) {
        SsVabClose(gp_midi_vab_id_2);
    }
}

s32 Midi_Control(AudioChannel *record, s32 action, s32 arg) {
    s32 i;

    if (record != NULL) {
        switch (action) {
        case 0:
            record->state = 1;
            SsSepPlay(gp_midi_vol, record->channel, 1, (s16)arg);
            break;
        case 1:
            record->state = 2;
            SsSepStop(gp_midi_vol, record->channel);
            break;
        case 2:
            record->state = 0;
            SsSepPause(gp_midi_vol, record->channel);
            break;
        case 3:
            record->state = 1;
            SsSepReplay(gp_midi_vol, record->channel);
            break;
        case 4:
            record->left_volume = arg >> 16;
            record->right_volume = arg;
            for (i = (gp_midi_open_seq_cnt >= 2); i < gp_midi_open_seq_cnt; i++) {
                SsSepSetVol(gp_midi_vol, record->channel, record->left_volume, record->right_volume);
            }
            break;
        }
    }
}
