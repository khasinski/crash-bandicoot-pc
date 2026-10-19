#include "common.h"
#include "audio_channels.h"

void Midi_Init(void) {
    s16 channel;
    s32 scaled_gain;
    s32 index;
    s32 voice_limit;
    u32 gain;

    SsSetTickMode(1);
    index = gp_midi_open_seq_cnt >= 2;
    gain = ((u32)g_levels_unlocked[0] * 0x3FFFU) >> 8;
    gp_midi_seq_gain = (s16) gain;
    scaled_gain = (s32) (gain << 0x10) >> 0x17;
    if (index < gp_midi_open_seq_cnt) {
        s32 volume = scaled_gain;
        do {
            channel = g_midi_channel_states[index].channel;
            SsSepSetVol(gp_midi_vol, channel, volume, volume);
            index += 1;
        } while (index < gp_midi_open_seq_cnt);
    }
    gp_midi_loaded_eid = 0x6396347F;
    gp_card_chan = 0;
    gp_midi_vol = -1;
    gp_midi_vab_id_2 = -1;
    if (g_levels_unlocked[0] == 0) {
        voice_limit = 8;
    } else if (g_cur_map_level[0] == 0) {
        voice_limit = 20;
    } else {
        switch (gp_pbak_mode) {
        case 0x3:
            voice_limit = 0xB;
            break;
        case 0x5:
        case 0xC:
        case 0x15:
            voice_limit = 0xD;
            break;
        case 0x6:
        case 0x7:
            voice_limit = 0xE;
            break;
        case 0x1F:
        case 0x22:
        case 0x2E:
            voice_limit = 0xF;
            break;
        case 0xE:
        case 0x13:
            voice_limit = 0x11;
            break;
        case 0x8:
        case 0x11:
        case 0x14:
        case 0x16:
        case 0x1E:
        case 0x24:
        case 0x28:
        case 0x2A:
        case 0x33:
            voice_limit = 0x12;
            break;
        case 0xA:
            voice_limit = 0x15;
            break;
        default:
            voice_limit = 0x10;
            break;
        }
    }
    gp_sfx_voice_limit = voice_limit;
    SsSetReservedVoice(*(u8 *)&gp_sfx_voice_limit);
}
