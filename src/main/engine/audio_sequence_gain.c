#include "common.h"
#include "audio_channels.h"

void Midi_ResetSeqVol(s16 gain) {
    s16 channel;
    s32 scaled_gain;
    s32 index;

    gp_midi_seq_gain = gain;
    index = gp_midi_open_seq_cnt >= 2;
    scaled_gain = gain >> 7;
    if (index < gp_midi_open_seq_cnt) {
        s32 volume = scaled_gain;
        do {
            channel = g_midi_channel_states[index].channel;
            SsSepSetVol(gp_midi_vol, channel, volume, volume);
            index += 1;
        } while (index < gp_midi_open_seq_cnt);
    }
}
