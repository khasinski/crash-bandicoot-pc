#include "common.h"
#include "voice_attributes.h"

static inline u16 encode_attack(s32 mode) {
    switch (mode) {
    case 1: return 0;
    case 5: return 128;
    default: return 0;
    }
}
static inline u16 encode_sustain(s32 mode) {
    switch (mode) {
    case 1: return 0;
    case 3: return 256;
    case 5: return 512;
    case 7: return 768;
    default: return 256;
    }
}
static inline u16 encode_release(s32 mode) {
    switch (mode) {
    case 3: return 0;
    case 7: return 32;
    default: return 0;
    }
}
static inline u16 clamp_rate4(const u16 *rate) {
    u16 value = *rate;
    if (value < 16) return *rate;
    return 15;
}
s32 _SpuSetVoiceAttr(VoiceAttributes *attr, s32 first, s32 last, s32 skip_wait) {
    volatile s32 delay_count;
    volatile s32 delay_product;
    /* ADSR words and the MMIO base occupy this scratch value in separate phases. */
    union { u32 word; volatile u16 *base; } operand;
    u32 voice_address;
    s16 left_input;
    s16 right_input;
    s16 right_mode;
    s16 left_mode;
    s32 field_mask;
    s32 all_fields;
    s32 word_index;
    s32 invalid_range;
    s32 first_voice;
    u16 adsr_mode;
    u32 volume_mode;
    s32 voice;
    s32 end_voice;
    s32 skip_delay;
    s32 encoded_volume;
    u16 base_note;
    u16 note;
    u16 rate;
    u16 right_volume;
    u16 left_volume;
    volatile u16 *attack_regs;
    volatile u16 *decay_regs;
    volatile u16 *level_regs;
    volatile u16 *sustain_regs;
    volatile u16 *release_regs;

    first_voice = first;
    end_voice = last;
    skip_delay = skip_wait;
    left_volume = 0;
    field_mask = attr->mask;
    right_volume = 0;
    all_fields = field_mask == 0;
    if (first_voice < 0) {
        first_voice = 0;
    }
    if (first_voice >= 24) goto invalid;
    if (end_voice >= 24) end_voice = 23;
    if (end_voice < 0) goto invalid;
    invalid_range = end_voice < first_voice;
    end_voice++;
    if (!invalid_range) goto valid;
invalid:
return -3;
valid:
    voice = first_voice;
    if (voice < end_voice) {
        do {
            if ((1 << voice) & attr->voice) {
                word_index = voice * 8;
                if ((all_fields != 0) || (field_mask & 0x10)) {
                    g_spu_RXX[(voice * 8) + 2] = (u16) attr->pitch;
                }
                if ((all_fields != 0) || (field_mask & 0x40)) {
                    g_spu_pitch_scale_tab[voice] = attr->sample_note;
                }
                if ((all_fields != 0) || ((field_mask & 0x20) != 0)) {
                    base_note = g_spu_pitch_scale_tab[voice];
                    note = attr->note;
                    g_spu_RXX[word_index + 2] = _spu_note2pitch(base_note >> 8, base_note & 0xFF, note >> 8, note & 0xFF);
                }
                if ((all_fields != 0) || (field_mask & 0x80)) {
                    _spu_FsetRXXa(word_index | 3, attr->addr);
                }
                if (all_fields == 0) {
                    if (field_mask & 0x800) {
                        adsr_mode = 0;
                        if (field_mask & 0x100) {
                            goto load_attack_mode;
                        }
                        goto set_attack_rate;
                    }
                } else {
load_attack_mode:
                    adsr_mode = encode_attack(attr->a_mode);
set_attack_rate:
                    rate = 0x7F;
                    if ((u16) attr->ar < 0x80U) {
                        rate = attr->ar;
                    }
                    attack_regs = g_spu_RXX + word_index;
                    operand.word = attack_regs[4] & 0xFF;
                    attack_regs[4] = (u16) (operand.word | ((rate | adsr_mode) << 8));
                }
                if ((all_fields != 0) || (field_mask & 0x1000)) {
                    rate = clamp_rate4(&attr->dr);
                    decay_regs = g_spu_RXX + word_index;
                    operand.word = decay_regs[4] & 0xFF0F;
                    decay_regs[4] = (u16) (operand.word | (rate * 0x10));
                }
                if ((all_fields != 0) || (field_mask & 0x8000)) {
                    rate = clamp_rate4(&attr->sl);
                    level_regs = g_spu_RXX + word_index;
                    level_regs[4] = (u16) (rate | (level_regs[4] & 0xFFF0));
                }
                if (all_fields == 0) {
                    if (field_mask & 0x2000) {
                        adsr_mode = 0;
                        if (field_mask & 0x200) {
                            goto load_sustain_mode;
                        }
                        goto set_sustain_rate;
                    }
                } else {
load_sustain_mode:
                    adsr_mode = encode_sustain(attr->s_mode);
set_sustain_rate:
                    rate = 0x7F;
                    if ((u16) attr->sr < 0x80U) {
                        rate = attr->sr;
                    }
                    sustain_regs = g_spu_RXX + word_index;
                    operand.word = sustain_regs[5] & 0x3F;
                    sustain_regs[5] = (u16) (operand.word | ((rate | adsr_mode) << 6));
                }
                if (all_fields == 0) {
                    if (field_mask & 0x4000) {
                        adsr_mode = 0;
                        if (field_mask & 0x400) {
                            goto load_release_mode;
                        }
                        goto set_release_rate;
                    }
                } else {
load_release_mode:
                    adsr_mode = encode_release(attr->r_mode);
set_release_rate:
                    rate = 0x1F;
                    if ((u16) attr->rr < 0x20U) {
                        rate = attr->rr;
                    }
                    release_regs = g_spu_RXX + word_index;
                    operand.word = release_regs[5] & 0xFFC0;
                    release_regs[5] = (u16) (operand.word | (rate | adsr_mode));
                }
                if ((all_fields != 0) || ((field_mask & 0x20000) != 0)) {
                    operand.word = attr->adsr1;
                    g_spu_RXX[word_index + 4] = (u16) operand.word;
                }
                if ((all_fields != 0) || ((field_mask & 0x40000) != 0)) {
                    operand.word = attr->adsr2;
                    g_spu_RXX[word_index + 5] = (u16) operand.word;
                }
                if ((all_fields != 0) || (field_mask & 0x10000)) {
                    _spu_FsetRXXa(word_index | 7, attr->loop_addr);
                }
                if ((all_fields != 0) || (field_mask & 1)) {
                    if ((all_fields != 0) || (field_mask & 4)) {
                        left_mode = attr->volmode_left;
                        switch (left_mode) {
                        case 1:
                            volume_mode = 0x8000;
                            break;
                        case 2:
                            volume_mode = 0x9000;
                            break;
                        case 3:
                            volume_mode = 0xA000;
                            break;
                        case 4:
                            volume_mode = 0xB000;
                            break;
                        case 5:
                            volume_mode = 0xC000;
                            break;
                        case 6:
                            volume_mode = 0xD000;
                            break;
                        case 7:
                            volume_mode = 0xE000;
                            break;
                        case 0:
                        default:
                            left_volume = attr->vol_left;
                            volume_mode = 0;
                            break;
                        }
                    } else {
                        left_volume = attr->vol_left;
                        volume_mode = 0;
                    }
                    if (volume_mode != 0) {
                        left_input = (s16) attr->vol_left;
                        if (left_input >= 0x80) {
                            left_volume = 0x7F;
                        } else {
                            left_volume = (u16) left_input;
                            if (left_input < 0) {
                                left_volume = 0;
                            }
                        }
                    }
                    encoded_volume = left_volume & 0x7FFF;
                    voice_address = word_index * 2;
                    operand.base = g_spu_RXX;
                    voice_address += (u32)operand.base;
                    *(volatile u16 *)(voice_address + 0) = encoded_volume | volume_mode;
                }
                if ((all_fields != 0) || (field_mask & 2)) {
                    if ((all_fields != 0) || (field_mask & 8)) {
                        right_mode = attr->volmode_right;
                        switch (right_mode) {
                        case 1:
                            volume_mode = 0x8000;
                            break;
                        case 2:
                            volume_mode = 0x9000;
                            break;
                        case 3:
                            volume_mode = 0xA000;
                            break;
                        case 4:
                            volume_mode = 0xB000;
                            break;
                        case 5:
                            volume_mode = 0xC000;
                            break;
                        case 6:
                            volume_mode = 0xD000;
                            break;
                        case 7:
                            volume_mode = 0xE000;
                            break;
                        case 0:
                        default:
                            right_volume = attr->vol_right;
                            volume_mode = 0;
                            break;
                        }
                    } else {
                        right_volume = attr->vol_right;
                        volume_mode = 0;
                    }
                    if (volume_mode != 0) {
                        right_input = (s16) attr->vol_right;
                        if (right_input >= 0x80) {
                            right_volume = 0x7F;
                        } else {
                            right_volume = (u16) right_input;
                            if (right_input < 0) {
                                right_volume = 0;
                            }
                        }
                    }
                    encoded_volume = right_volume & 0x7FFF;
                    voice_address = word_index * 2;
                    operand.base = g_spu_RXX;
                    voice_address += (u32)operand.base;
                    ((volatile u16 *)voice_address)[1] = encoded_volume | volume_mode;
                }
            }
            voice += 1;
        } while (voice < end_voice);
    }
    /* Normalize the cached flag before the optional SDK wait. */
    skip_delay = skip_delay != 0;
    if (skip_delay == 0) {
        delay_product = 1;
        delay_count = 0;
        while (delay_count < 2) {
            delay_product *= 13;
            delay_count++;
        }
    }
    return 0;
}
