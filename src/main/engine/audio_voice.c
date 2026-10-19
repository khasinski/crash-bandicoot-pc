#include "common.h"

/* =========================================================================
 * Section 2: Audio / SPU Voice Management (asm/USA/main/text_20878.s)
 * ========================================================================= */

typedef struct AudioVolume {
    s16 left;
    s16 right;
} AudioVolume;

typedef struct Vec3 {
    s32 x;
    s32 y;
    s32 z;
} Vec3;

typedef struct GoolObjectAudio {
    u8 reserved_00[0x80];
    Vec3 trans;
    u8 reserved_8c[0x40];
    u32 status_b;
} GoolObjectAudio;

typedef struct VoiceParams {
    u32 b0 : 1;
    u32 b1 : 1;
    u32 b2 : 1;
    u32 b3 : 1;
    u32 b4 : 1;
    u32 b5 : 1;
    u32 b6 : 1;
    u32 b7 : 1;
    u32 b8 : 1;
    u32 b9 : 1;
    u32 b10 : 1;
    u32 b11 : 1;
    s32 word_04;
    RAM_REF(GoolObjectAudio) obj;
    u8 word_0c;
    u8 priority;
    s16 volume;
    s16 delay;
    u16 pitch;
    Vec3 trans_pos;
    Vec3 world_pos;
    s16 target_vol;
    u16 target_pitch;
    s32 vol_ticks;
    s32 vol_step;
    s32 pitch_ticks;
    s32 pitch_step;
} VoiceParams;

typedef struct VoiceEntry {
    s32 id;
    VoiceParams params;
} VoiceEntry;


typedef struct LevelHeaderInfo {
    s32 word_00;
    s32 id;
} LevelHeaderInfo;

#define CRASH_EXT_ENGINE_AUDIO_VOICE_0
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_AUDIO_VOICE_0
extern s32 gp_midi_state;
extern s32 gp_sfx_vol;
extern s16 gp_mus_vol;
extern s16 gp_sfx_vol_scaled;
extern s16 gp_mus_vol_scaled;

#define CRASH_EXT_ENGINE_AUDIO_VOICE_1
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_AUDIO_VOICE_1
typedef struct ZonePtr57914 {
    u8 reserved_00[0x10];
    s32 word_10;
} ZonePtr57914;
#define CRASH_EXT_ENGINE_AUDIO_VOICE_2
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_AUDIO_VOICE_2

/* A voice's object, or for a voice without one the word the PS1 read at
 * address 0 + offset (low RAM). */
#define VOICE_OBJ(word) RAM_AT(GoolObjectAudio, (word) != 0 ? (word) : RAM_PS1_BASE)

static __inline__ void audio_set_volume(SpuVolume *dst, AudioVolume *src) {
    dst->left = src->left;
    dst->right = src->right;
}

AudioVolume *Audio_Spatialize(Vec3 *pos, s32 vol) {
    s32 x, z, y, dist, pan, base_vol, left, right;

    x = pos->x / 32;
    z = (pos->z - 0x4B000) / 32;
    if (RAM_AT(LevelHeaderInfo, g_ns_page_pool)->id == 0x22 || RAM_AT(LevelHeaderInfo, g_ns_page_pool)->id == 0x2E) {
        y = pos->y / 32;
        dist = RSqrMagnitude3(x, z, y);
        RSqrMagnitude2(x, z);
    } else {
        dist = RSqrMagnitude2(x, z);
    }
    if (dist <= 0x7FFF) {
        gp_mus_vol = vol;
        gp_sfx_vol_scaled = vol;
    } else {
        pan = Math_Cos(ratan2(z, x)) + 0x1000;
        base_vol = (vol << 15) / dist;
        if (g_stereo_disabled[0] == 0) {
            left = (base_vol + base_vol * (0x2000 - pan)) >> 11;
            right = (base_vol + base_vol * pan) >> 11;
        } else {
            left = right = base_vol;
        }
        gp_mus_vol = left >= vol ? vol : (left < 0 ? 0 : left);
        gp_sfx_vol_scaled = right >= vol ? vol : (right < 0 ? 0 : right);
    }
    return (AudioVolume *)&gp_mus_vol;
}

void Audio_VoiceFree(GoolObjectAudio *obj) {
    s32 i;
    s32 zero;
    VoiceEntry *entry;

    if (obj != NULL) {
        i = gp_sfx_voice_limit;
        entry = &g_voices[i];
        for (; i < 24; i++, entry++) {
            if ((entry->params.b3) && entry->params.obj == Ram_Addr(obj)) {
                zero = 0;
                gp_midi_state = 9;
                Audio_Control(entry->id, 0x40000000, &zero, RAM_AT(GoolObjectAudio, entry->params.obj));
                entry->params.obj = 0;
                entry->params.priority = 0;
            }
        }
    }
}

s32 Audio_VoiceAlloc(void) {
    SpuVoiceAttr attr;
    s32 min_vol = 0x3FFF;
    s32 min_prio = 0xFF;
    s32 best_idx;
    VoiceEntry *entry;
    s32 i;
    s32 left, right, vol;

    best_idx = gp_sfx_voice_limit;
    entry = &g_voices[best_idx];
    for (; best_idx < 24; best_idx++, entry++) {
        if (entry->params.priority < min_prio) {
            min_prio = entry->params.priority;
        }
        if (!entry->params.b3) {
            return best_idx;
        }
    }
    if (g_audio_voices_end.priority < min_prio) {
        return -1;
    }
    i = gp_sfx_voice_limit;
    entry = &g_voices[i];
    best_idx = -1;
    for (; i < 24; i++, entry++) {
        if (entry->params.priority == min_prio) {
            attr.voice = 1 << i;
            SpuGetVoiceAttr(&attr);
            left = attr.volume.left;
            right = attr.volume.right;
            vol = left < 0 ? -left : left;
            right = right < 0 ? -right : right;
            if (vol < right) {
                vol = right;
            }
            if (vol < min_vol) {
                best_idx = i;
                min_vol = vol;
            }
        }
    }
    if (min_prio == g_audio_voices_end.priority && min_vol >= 0x800 && Math_Randb(100) >= 50) {
        return -1;
    }
    return best_idx;
}

s32 Audio_VoiceCreate(GoolObjectAudio *obj, u32 *ref, s32 vol) {
    SpuVoiceAttr attr;
    Vec3 pos;
    VoiceEntry *entry;
    s32 voice_idx;
    s32 res;
    u32 addr;

    if (g_cur_map_level[0] == 0) {
        return 0;
    }
    voice_idx = Audio_VoiceAlloc();
    if (voice_idx > 0) {
        entry = &g_voices[voice_idx];
        /* A tagged argument is an SPU address itself; native callers always
         * pass a pointer to the sound's EID. */
        if ((uintptr_t)ref & 1) {
            addr = (u32)((uintptr_t)ref & ~(uintptr_t)1);
        } else {
            res = Ns_Lookup(ref);
            if (res >= -0xFE) {
                return -1;
            }
            addr = ((u32)res << 1) >> 3;
        }
        entry->params = g_audio_voices_end;
        entry->params.volume = (vol * gp_mus_vol_scaled) >> 14;
        if (entry->params.b6) {
            entry->params.vol_step = (entry->params.target_vol - entry->params.volume) / entry->params.vol_ticks;
        }
        entry->params.obj = Ram_Addr(obj);
        if (obj != NULL) {
            entry->params.world_pos = obj->trans;
            Gool_Transform2(&obj->trans, &pos, 1);
            entry->params.trans_pos = pos;
        }
        g_audio_voices_end.word_0c = 1;
        g_audio_voices_end.priority = 0x80;
        g_audio_voices_end.volume = 0x3FFF;
        g_audio_voices_end.pitch = 0x1000;
        g_audio_voices_end.obj = 0;
        g_audio_voices_end.delay = 0;
        g_audio_voices_end.trans_pos.x = 0;
        g_audio_voices_end.trans_pos.y = 0;
        g_audio_voices_end.trans_pos.z = 0;
        g_audio_voices_end.b1 = 0;
        g_audio_voices_end.b0 = 0;
        g_audio_voices_end.b2 = 0;
        g_audio_voices_end.b3 = 0;
        g_audio_voices_end.b4 = 0;
        g_audio_voices_end.b5 = 0;
        g_audio_voices_end.b6 = 0;
        g_audio_voices_end.b7 = 0;
        g_audio_voices_end.b8 = 0;
        g_audio_voices_end.b9 = 1;
        g_audio_voices_end.b10 = 1;
        g_audio_voices_end.b11 = 0;
        attr.mask = 0xFF93;
        attr.voice = 1 << voice_idx;
        if (VOICE_OBJ(entry->params.obj)->status_b & 0x200) {
            attr.volume.left = attr.volume.right = entry->params.volume;
            entry->params.b9 = 0;
        } else {
            audio_set_volume(&attr.volume, Audio_Spatialize(&entry->params.trans_pos, entry->params.volume));
        }
        attr.pitch = entry->params.pitch;
        attr.addr = addr;
        attr.a_mode = 1;
        attr.s_mode = 1;
        attr.r_mode = 3;
        attr.ar = 0;
        attr.dr = 0;
        attr.sr = 0;
        attr.rr = 0;
        attr.sl = 0xF;
        SpuSetVoiceAttr(&attr);
        SpuSetReverbVoice(entry->params.b10, attr.voice);
        if (!entry->params.b4) {
            SpuSetKey(1, attr.voice);
        }
        entry->id = ++gp_sfx_vol;
        entry->params.b3 = 1;
        return entry->id;
    }
    return -1;
}

s32 Audio_Control(s32 id, u32 op, void *arg2, GoolObjectAudio *obj) {
    SpuVoiceAttr s_attr;
    u8 c_attr[0x28];
    Vec3 v1;
    Vec3 v2;
    s32 i;
    VoiceEntry *entry;
    VoiceParams *params;

    i = gp_sfx_voice_limit;
    entry = &g_voices[i];
    for (; i < 24; i++, entry++) {
        if ((id == 0 || (id == -1 || entry->id == id)) && (id != -1 || entry->params.obj == Ram_Addr(obj))) {
            params = &g_audio_voices_end;
            if (id != 0) {
                params = &entry->params;
            }
            if (op & 0x80000000) {
                params->b0 = 1;
            }
            if (op & 0x40000000) {
                params->b1 = 1;
            }
            if (op & 0x20000000) {
                params->b2 = 1;
            }
            s_attr.mask = 0;
            switch (op & 0xFFFFFFF) {
            case 0:
                if (params->b2) {
                    params->target_vol = *(s16 *)arg2;
                    params->vol_ticks = gp_midi_state;
                    if (id != 0) {
                        params->vol_step = (params->target_vol - params->volume) / params->vol_ticks;
                    }
                    params->b6 = 1;
                } else {
                    params->volume = *(s16 *)arg2;
                    if (id != 0) {
                        s_attr.mask |= 3;
                        audio_set_volume(&s_attr.volume, Audio_Spatialize(&params->trans_pos, params->volume));
                    }
                }
                break;
            case 1:
                if (params->b2) {
                    params->target_pitch = *(u16 *)arg2;
                    params->pitch_step = (params->target_pitch - params->pitch) / gp_midi_state;
                    params->pitch_ticks = gp_midi_state;
                    params->b7 = 1;
                } else {
                    params->pitch = *(u16 *)arg2;
                    if (id != 0) {
                        s_attr.mask = 0x10;
                        s_attr.pitch = params->pitch;
                    }
                }
                break;
            case 2:
                v1 = *(Vec3 *)arg2;
                Gool_Transform2(&v1, (Vec3 *)arg2, 1);
            case 3:
                v2 = *(Vec3 *)arg2;
                if (!params->b2) {
                    params->trans_pos = *(Vec3 *)arg2;
                    if (id != 0) {
                        entry->params.trans_pos = *(Vec3 *)arg2;
                        s_attr.mask = 3;
                        audio_set_volume(&s_attr.volume, Audio_Spatialize(&params->trans_pos, params->volume));
                    }
                }
                break;
            case 4:
                params->word_0c = *(u8 *)arg2;
                break;
            case 5:
                params->obj = *(u32 *)arg2;
                break;
            case 6:
                gp_midi_state = *(s32 *)arg2;
                if (gp_midi_state == 0) {
                    gp_midi_state = 1;
                }
                break;
            case 7:
                params->delay = *(s16 *)arg2;
                params->b4 = 1;
                break;
            case 8:
                params->b9 = 1;
                break;
            case 9:
                params->b9 = 0;
                break;
            case 10:
                params->b11 = *(s32 *)arg2 >> 8;
                break;
            case 11:
                params->b10 = *(s32 *)arg2 >> 8;
                break;
            case 12:
                params->priority = *(u8 *)arg2;
                break;
            }
            if (s_attr.mask != 0) {
                s_attr.voice = 1 << i;
                SpuSetVoiceAttr(&s_attr);
            }
            if (id != -1) {
                return 0;
            }
        }
    }
    return id == -1 ? 0 : -1;
}

void Audio_Update(void) {
    SpuVoiceAttr attr;
    Vec3 pos;
    s32 flag = 0;
    s32 i;
    VoiceEntry *entry;
    s32 v;
    u32 tmp;
    GoolObjectAudio *obj;

    tmp = RAM_AT(LevelHeaderInfo, g_ns_page_pool)->id;
    if (tmp == 0x19) {
        v = g_cur_map_level[0];
        tmp = v << 14;
        tmp -= v;
        tmp >>= 8;
        gp_mus_vol_scaled = tmp;
        v = g_levels_unlocked[0];
        Midi_ResetSeqVol((s16)((v * 0x3FFF) >> 8));
    }
    if (g_pads != 0) {
        Midi_Update(RAM_AT(s32, RAM_AT(ZonePtr57914, g_pads)->word_10 + 0x304));
    }
    if (gp_midi_fade_step != 0) {
        if (gp_midi_fade_step < 0 && gp_spu_master_vol < -gp_midi_fade_step) {
            gp_spu_master_vol = 0;
            gp_midi_fade_step = 0;
        }
        if (gp_midi_fade_step > 0 && 0x3FFF - gp_spu_master_vol < gp_midi_fade_step) {
            gp_spu_master_vol = 0x3FFF;
            gp_midi_fade_step = 0;
        }
        gp_spu_master_vol += gp_midi_fade_step;
        SsSetMVol(gp_spu_master_vol >> 7, gp_spu_master_vol >> 7);
    }
    if (gp_use_cd == 1) {
        gp_midi_fade_step = -0x222;
        if (gp_midi_default_rvol != 0 && gp_midi_open_seq_cnt >= 2 && gp_card_chan == 3) {
            Midi_Control((struct AudioChannel *)g_midi_seq_slot_b, 2, 0);
        }
    }
    if (gp_use_cd == -1) {
        gp_midi_fade_step = 0x667;
        if (gp_midi_default_rvol != 0 && gp_midi_open_seq_cnt >= 2 && gp_card_chan == 3) {
            Midi_Control((struct AudioChannel *)g_midi_seq_slot_b, 3, 0);
        }
    }
    SpuGetAllKeysStatus(g_spu_key_statuses);
    i = gp_sfx_voice_limit;
    entry = &g_voices[i];
    for (; i < 24; i++, entry++) {
        if (entry->params.b3) {
            if (entry->params.b4) {
                if (--entry->params.delay != 0) {
                    continue;
                }
                entry->params.b4 = 0;
                SpuSetKey(1, 1 << i);
            }
            if (g_spu_key_statuses[i] == 3) {
                if (--entry->params.word_0c == 0) {
                    SpuSetKey(0, 1 << i);
                    entry->params.b3 = 0;
                    goto check_end;
                } else {
                    SpuSetKey(1, 1 << i);
                }
            }
            attr.mask = 0;
            if (entry->params.b6) {
                entry->params.volume += entry->params.vol_step;
                if (--entry->params.vol_ticks > 0) {
                    flag = 1;
                } else {
                    entry->params.b6 = 0;
                }
                attr.mask |= 3;
                if (VOICE_OBJ(entry->params.obj)->status_b & 0x200) {
                    attr.volume.left = attr.volume.right = entry->params.volume;
                } else {
                    audio_set_volume(&attr.volume, Audio_Spatialize(&entry->params.trans_pos, entry->params.volume));
                }
            }
            if (entry->params.b7) {
                entry->params.pitch += entry->params.pitch_step;
                if (--entry->params.pitch_ticks > 0) {
                    flag = 1;
                } else {
                    entry->params.b7 = 0;
                }
                attr.mask |= 0x10;
                attr.pitch = entry->params.pitch;
            }
            if (entry->params.b9 && (obj = RAM_AT(GoolObjectAudio, entry->params.obj)) != NULL) {
                entry->params.world_pos = obj->trans;
                Gool_Transform2(&obj->trans, &pos, 1);
                entry->params.trans_pos = pos;
                attr.mask |= 3;
                audio_set_volume(&attr.volume, Audio_Spatialize(&entry->params.trans_pos, entry->params.volume));
            }
            if (attr.mask != 0) {
                attr.voice = 1 << i;
                SpuSetVoiceAttr(&attr);
            }
        check_end:
            if (entry->params.b0 || (!flag && entry->params.b1)) {
                SpuSetKey(0, 1 << i);
                entry->params.b3 = 0;
            }
        }
    }
}
