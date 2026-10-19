#include "common.h"

s32 _spu_note2pitch(s32 base_note, s32 base_fine, s32 note, s32 fine) {
    s16 octave_shift;
    s16 semitone;
    s16 fine_steps;
    s16 next_steps;
    s16 fraction;
    s32 combined_fine;
    s32 carry;
    u16 pitch;

    combined_fine = (fine & 0xFFFF) + (base_fine & 0xFFFF);
    if (combined_fine < 0) {
        combined_fine += 7;
    }
    fine_steps = combined_fine >> 3;
    fraction = fine_steps;
    carry = 0;
    if (fine_steps >= 0x10) {
        do {
            next_steps = fraction - 0x10;
            fraction = next_steps;
            carry += 1;
        } while ((s16) next_steps >= 0x10);
    }
    if ((s16)fraction < 0) {
        fraction = 0;
    }
    semitone = carry + (60 - base_note + note);
    pitch = g_spu_semitone_pitch_tab[((s16)(semitone % 12) << 4) + (s16)fraction];
    octave_shift = (semitone / 12) - 5;
    if (octave_shift > 0) {
        pitch <<= octave_shift;
    } else if (octave_shift < 0) {
        pitch = (u16) ((s32) (pitch & 0xFFFF) >> -octave_shift);
    }
    return pitch & 0xFFFF;
}
