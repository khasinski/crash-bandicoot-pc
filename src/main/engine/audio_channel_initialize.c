#include "common.h"
#include "audio_channels.h"

void Midi_SeqInit(AudioChannel *record, s32 packed_volume, s32 channel) {
    if (gp_midi_open_seq_cnt >= 2 && channel == 1) {
        record->right_volume = (u16) gp_midi_default_rvol;
        record->left_volume = (u16) gp_midi_default_rvol;
    } else {
        record->left_volume = packed_volume >> 16;
        record->right_volume = packed_volume;
    }
    record->channel = channel;
    record->state = -1;
    SsSepSetVol(gp_midi_vol, (s16) channel, (s16) record->left_volume, (s16) record->right_volume);
}
