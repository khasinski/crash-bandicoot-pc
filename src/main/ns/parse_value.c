#include "common.h"

static __inline__ s32 ns_alpha_num_index(u8 ch)
{
    s32 index;
    for (index = 0; index < 0x40; index++) {
        if (g_eid_charset_table[index] == ch)
            return index;
    }
    if (ch == 0x2D)
        return 0x63;
    return -0xE;
}

s32 Ns_Atoi(u32 type, u8 *text) {
    s32 end_address;
    s32 packed;
    s32 alphabet_index;
    u8 *cursor;
    u32 character;

    switch (type) {
    case 3:
        packed = 0;
        cursor = text;
        end_address = 5;
        do {
            packed <<= 6;
            alphabet_index = 0;
            character = *cursor;
search_packed_character:
            if (g_eid_charset_table[alphabet_index] == character) {
                packed |= alphabet_index;
            } else {
                alphabet_index += 1;
                if (alphabet_index < 0x40) {
                    goto search_packed_character;
                }
            }
            cursor += 1;
        } while (cursor < text + end_address);
        packed *= 2;
        return packed | 1;
    case 2:
        return (BIOS_AToI(text) * 2) | 1;
    case 0:
    case 1:
    case 13:
    case 14:
        return ns_alpha_num_index(*text);
    default:
        return BIOS_AToI(text);
    }
}

/* Returns the PS1 address of the formatted filename. */
s32 Ns_GetFilenameStr(u32 type, u8 *text) {
    return (s32)Ram_Addr(Ns_GetFilename2(type, Ns_Atoi(type, text), 0, 0));
}
