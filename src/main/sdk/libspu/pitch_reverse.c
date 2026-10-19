#include "common.h"

s32 _spu_pitch2note(s32 base_note, s32 base_fine, s32 pitch) {
    s32 remaining;
    s32 note;
    s32 range;
    s32 semitones;
    s32 octave_base;
    s32 fine;
    s32 highest;
    s32 fraction;
    s32 bit;

    highest = 0;
    bit = 0;
    do {
        if (((s32) (pitch & 0xFFFF) >> bit) & 1) {
            highest = bit;
        }
        bit += 1;
    } while (bit < 0x10);
    range = 1 << (highest + 1);
    octave_base = 1 << highest;
    range -= octave_base;
    remaining = (pitch & 0xFFFF) - octave_base;
    semitones = (s32) (remaining * 0xC) / range;
    note = (base_note & 0xFFFF) + ((highest - 0xC) * 0xC) + semitones;
    fine = (base_fine & 0xFFFF) + ((s32) ((remaining - ((range * semitones) / 12)) * 0x600) / range);
    fraction = fine % 128;
    note += (fine >= 0x80);
    if (note < 0) return -1;
    if (note >= 128) return -1;
    return (note << 8) | fraction;
}
