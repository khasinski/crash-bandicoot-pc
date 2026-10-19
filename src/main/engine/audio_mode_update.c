#include "common.h"
#include "audio_channels.h"

#define CRASH_EXT_ENGINE_AUDIO_MODE_UPDATE
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_AUDIO_MODE_UPDATE

void Midi_TogglePlayback(s32 request) {
    s32 mode;

    if (gp_midi_open_seq_cnt >= 2) {
        mode = request >> 8;
        if (gp_card_chan == 3) {
            if ((mode <= 0) || (mode < 3) || (mode != gp_card_chan)) {
                Midi_Control(g_midi_seq_slot_b, 1, 0);
                Midi_Control(g_midi_seq_slot_b - 1, 3, 0);
                gp_midi_default_rvol = 0;
            } else {
                if ((g_levels_unlocked[0] != 0) || (g_cur_map_level[0] != 0)) {
                    gp_midi_default_rvol = 0x3FFF;
                } else {
                    gp_midi_default_rvol = 0;
                }
                Midi_Control(g_midi_channel_states, 2, 0);
                Midi_Control(g_midi_channel_states + 1, 0, 1);
            }
            SsSepSetVol(gp_midi_vol, g_midi_seq_slot_b[0].channel, (s16) gp_midi_default_rvol, (s16) gp_midi_default_rvol);
        }
    }
}
