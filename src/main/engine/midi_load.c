#include "common.h"
#include "audio_channels.h"

typedef struct MidiHeaderItem {
    u16 count;
    u16 reserved_02;
    s32 bank_ref;
    s32 refs[7];
} MidiHeaderItem;

typedef struct MidiResourceEntry {
    u32 word00;
    s32 id;
    s32 kind;
    s32 word0C;
    RAM_REF(MidiHeaderItem) header;
    RAM_REF(void) vh;
    RAM_REF(void) sep;
} MidiResourceEntry;

typedef void noreturn_fn(s32);
volatile noreturn_fn BIOS_Abs;

s32 Midi_OpenAndPlay(s32 *arg0) {
    s32 sp10;
    s32 i;
    MidiResourceEntry *entry;
    s32 is_new;

    is_new = (gp_midi_vab_id_2 == -1);
    if (gp_midi_default_rvol != 0) {
        return -1;
    }
    if (gp_card_chan == 1) {
        if (gp_midi_loaded_eid != 0x6396347F) {
            SsSepClose(gp_midi_vol);
            sp10 = gp_midi_loaded_eid;
            Ns_Close(&sp10, 1);
        }
        if (*arg0 != 0x6396347F) {
            entry = Ns_Open(arg0, 1, 1);
            gp_midi_loaded_eid = entry->id;
            for (i = 0; i < 7; i++) {
                if (entry->header->refs[i] != 0x6396347F) {
                    Ns_Open(&entry->header->refs[i], 0, 1);
                }
            }
        } else {
            gp_midi_loaded_eid = *arg0;
            return 0;
        }
    }
    gp_card_chan = 2;
    entry = Ns_Open(arg0, 1, 0);
    gp_midi_vab_id = 0;
    for (i = 0; i < 7; i++) {
        if (entry->header->refs[i] != 0x6396347F) {
            if (Ns_Close(&entry->header->refs[i], 0) == 0) {
                return 0;
            }
        }
    }
    gp_card_chan = 3;
    gp_midi_open_seq_cnt = entry->header->count;
    if (gp_midi_vab_id_2 == -1) {
        Ns_Open(&entry->header->bank_ref, 1, 1);
        gp_midi_vab_id_2 = SsVabFakeHead(entry->vh, -1, (g_ns_audio_bank_idx[0] << 16) | 0x2000);
        if (gp_midi_vab_id_2 == -1) {
            BIOS_Abs(-1);
        }
        gp_midi_vab_id_2 = SsVabFakeBody(gp_midi_vab_id_2);
        if (gp_midi_vab_id_2 == -1) {
            BIOS_Abs(-1);
        }
        SsStart();
    }
    gp_midi_vol = SsSepOpen(entry->sep, gp_midi_vab_id_2, gp_midi_open_seq_cnt);
    if (gp_midi_vol == -1) {
        BIOS_Abs(-1);
    }
    for (i = 0; i < gp_midi_open_seq_cnt; i++) {
        if (is_new) {
            s32 vol = (s16)gp_midi_seq_gain >> 7;
            Midi_SeqInit(&g_midi_channel_states[i], vol | (vol << 16), i);
        } else {
            Midi_SeqInit(&g_midi_channel_states[i], 0, i);
            SsSepSetCrescendo(gp_midi_vol, g_midi_channel_states[i].channel, (s16)gp_midi_seq_gain >> 7, 30);
        }
    }
    return Midi_Control(g_midi_channel_states, 0, 0);
}
