#include "common.h"
/* This unit registers addresses; callbacks have different parameter lists. */
typedef void (*Callback)();

s16 SsSepOpen(void *data_address, s16 mode, s16 channel_count) {
    s16 next_channel;
    s16 candidate_id;
    s16 channel;
    s16 sequence_id;
    s32 bytes_consumed;
    s32 found;
    s32 cursor;

    cursor = (s32)data_address;
    if (g_ss_open_seq_mask == -1) {
        BIOS_Printf(g_str_ss_sep_open_err);
failed:
        return -1;
    }
    candidate_id = 0;
    found = 0;
    g_Snd_FnTable[0] = &_SsNoteOn;
    g_ss_fn_note_off = &_SsNoteOff;
    g_ss_fn_meta_event = &_SsGetMetaEvent;
    g_ss_fn_pitch_bend = &_SsSetPitchBend;
    g_ss_fn_control_change = &_SsGetSeqData;
    g_ss_fn_program_change = &_SsSetProgramChange;
    g_ss_cc_main_vol = &_SsContModulation;
    g_ss_cc_panpot = &_SsContPortaTime;
    g_ss_cc_damper = &_SsContPortamento;
    g_ss_cc_nrpn_lsb = &_SsContNrpn1;
    g_ss_cc_nrpn_msb = &_SsContNrpn2;
    g_ss_cc_rpn_lsb = &_SsContRpn1;
    g_ss_cc_rpn_msb = &_SsContRpn2;
    g_ss_cc_reverb = &_SsContResetAll;
    g_ss_cc_reset_all = &_SsContDataEntry;
    g_ss_cc_data_entry = &_SsSetControlChange;
    g_ss_nrpn_attr_fn_00[0] = &Cd_Stub_01;
    g_ss_nrpn_attr_fn_01 = &Cd_Stub_02;
    g_ss_nrpn_attr_fn_02 = &Cd_Stub_03;
    g_ss_nrpn_attr_fn_03 = &Cd_Stub_04;
    g_ss_nrpn_attr_fn_04 = &Cd_Stub_05;
    g_ss_nrpn_attr_fn_05 = &Cd_Stub_06;
    g_ss_nrpn_attr_fn_06 = &Cd_Stub_07;
    g_ss_nrpn_attr_fn_07 = &Cd_Stub_08;
    g_ss_nrpn_attr_fn_08 = &Cd_Stub_09;
    g_ss_nrpn_attr_fn_09 = &Cd_Stub_10;
    g_ss_nrpn_attr_fn_10 = &Cd_Stub_11;
    g_ss_nrpn_attr_fn_11 = &Cd_Stub_12;
    g_ss_nrpn_attr_fn_12 = &Cd_Stub_13;
    g_ss_nrpn_attr_fn_13 = &Cd_Stub_14;
    g_ss_nrpn_attr_fn_14 = &Cd_Stub_15;
    g_ss_nrpn_attr_fn_15 = &Cd_Stub_16;
    g_ss_nrpn_attr_fn_16 = &Cd_Stub_17;
    g_ss_nrpn_attr_fn_17 = &Cd_Stub_18;
    g_ss_nrpn_attr_fn_18 = &Cd_Stub_19;
    g_ss_nrpn_attr_fn_19 = &Cd_Stub_20;
    do {
        if (!((1 << candidate_id) & g_ss_open_seq_mask)) {
            sequence_id = candidate_id;
            found = 1;
        }
        candidate_id += 1;
    } while ((found & 255) == 0);
    g_ss_open_seq_mask |= 1 << sequence_id;
    channel = 0;
    if (channel_count > 0) {
        do {
            bytes_consumed = _SsInitSoundSep(sequence_id, channel, mode, (u8 *)cursor);
            if (bytes_consumed == -1) goto failed;
            next_channel = channel + 1;
            channel = next_channel;
            cursor += bytes_consumed;
        } while (next_channel < channel_count);
    }

    return sequence_id;
}
