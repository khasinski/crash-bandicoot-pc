#include "common.h"

/* Input layout recovered from the retail register writes. */
typedef struct {
    s32 mask;
    u16 left, right;
    s16 left_mode, right_mode;
    u16 reverb_left, reverb_right;
    u16 cd_left, cd_right;
    s32 cd_reverb, cd_mix;
    u16 ext_left, ext_right;
    s32 ext_reverb, ext_mix;
} CommonAttributes;

void SpuSetCommonAttr(CommonAttributes *attr) {
    volatile u16 *control_registers;
    s16 right_input;
    s16 left_input;
    s32 left_mode;
    s32 right_mode;
    s16 cd_reverb_word;
    s16 cd_mix_word;
    s16 ext_reverb_word;
    s16 ext_mix_word;
    s32 field_mask;
    s32 all_fields;
    s32 left_encoding;
    s32 right_encoding;
    s32 left_word;
    s32 right_word;
    u16 left_volume;
    u16 right_volume;

    left_volume = 0;
    field_mask = attr->mask;
    all_fields = field_mask == 0;
    right_volume = 0;
    if ((all_fields != 0) || (field_mask & 1)) {
        if ((all_fields != 0) || (field_mask & 4)) {
        left_mode = attr->left_mode;
        switch (left_mode) {

        case 1:
            left_encoding = 0x8000;
            break;
        case 2:
            left_encoding = 0x9000;
            break;
        case 3:
            left_encoding = 0xA000;
            break;
        case 4:
            left_encoding = 0xB000;
            break;
        case 5:
            left_encoding = 0xC000;
            break;
        case 6:
            left_encoding = 0xD000;
            break;
        case 7:
            left_encoding = 0xE000;
            break;
        case 0:
        default:
            left_volume = attr->left;
            left_encoding = 0;
            break;
        }
        } else {
            left_volume = attr->left;
            left_encoding = 0;
        }
        if (left_encoding != 0) {
            left_input = (s16) attr->left;
            if (left_input >= 0x80) {
                left_volume = 0x7F;
            } else {
                left_volume = left_input;
                if (left_input < 0) {
                    left_volume = 0;
                }
            }
            left_word = left_volume & 0x7FFF;
        } else {
            left_word = left_volume & 0x7FFF;
        }
        g_spu_RXX[0x180 / 2] = (s16) (left_word | left_encoding);
    }
    if ((all_fields != 0) || (field_mask & 2)) {
        if ((all_fields != 0) || (field_mask & 8)) {
        right_mode = attr->right_mode;
        switch (right_mode) {

        case 1:
            right_encoding = 0x8000;
            break;
        case 2:
            right_encoding = 0x9000;
            break;
        case 3:
            right_encoding = 0xA000;
            break;
        case 4:
            right_encoding = 0xB000;
            break;
        case 5:
            right_encoding = 0xC000;
            break;
        case 6:
            right_encoding = 0xD000;
            break;
        case 7:
            right_encoding = 0xE000;
            break;
        case 0:
        default:
            right_volume = attr->right;
            right_encoding = 0;
            break;
        }
        } else {
            right_volume = attr->right;
            right_encoding = 0;
        }
        if (right_encoding != 0) {
            right_input = (s16) attr->right;
            if (right_input >= 0x80) {
                right_volume = 0x7F;
            } else {
                right_volume = right_input;
                if (right_input < 0) {
                    right_volume = 0;
                }
            }
            right_word = right_volume & 0x7FFF;
        } else {
            right_word = right_volume & 0x7FFF;
        }
        g_spu_RXX[0x182 / 2] = (s16) (right_word | right_encoding);
    }
    if ((all_fields != 0) || (field_mask & 0x40)) {
        g_spu_RXX[0x1B0 / 2] = (u16) attr->cd_left;
    }
    if ((all_fields != 0) || (field_mask & 0x80)) {
        g_spu_RXX[0x1B2 / 2] = (u16) attr->cd_right;
    }
    if ((all_fields != 0) || (field_mask & 0x100)) {
        if (attr->cd_reverb == 0) {
            control_registers = g_spu_RXX;
            cd_reverb_word = control_registers[0x1AA / 2];
            cd_reverb_word &= 0xFFFB;
        } else {
            control_registers = g_spu_RXX;
            cd_reverb_word = control_registers[0x1AA / 2];
            cd_reverb_word |= 4;
        }
        control_registers[0x1AA / 2] = cd_reverb_word;
    }
    if ((all_fields != 0) || (field_mask & 0x200)) {
        if (attr->cd_mix == 0) {
            control_registers = g_spu_RXX;
            cd_mix_word = control_registers[0x1AA / 2];
            cd_mix_word &= 0xFFFE;
        } else {
            control_registers = g_spu_RXX;
            cd_mix_word = control_registers[0x1AA / 2];
            cd_mix_word |= 1;
        }
        control_registers[0x1AA / 2] = cd_mix_word;
    }
    if ((all_fields != 0) || (field_mask & 0x400)) {
        g_spu_RXX[0x1B4 / 2] = (u16) attr->ext_left;
    }
    if ((all_fields != 0) || (field_mask & 0x800)) {
        g_spu_RXX[0x1B6 / 2] = (u16) attr->ext_right;
    }
    if ((all_fields != 0) || (field_mask & 0x1000)) {
        if (attr->ext_reverb == 0) {
            control_registers = g_spu_RXX;
            ext_reverb_word = control_registers[0x1AA / 2];
            ext_reverb_word &= 0xFFF7;
        } else {
            control_registers = g_spu_RXX;
            ext_reverb_word = control_registers[0x1AA / 2];
            ext_reverb_word |= 8;
        }
        control_registers[0x1AA / 2] = ext_reverb_word;
    }
    if ((all_fields != 0) || (field_mask & 0x2000)) {
        if (attr->ext_mix == 0) {
            control_registers = g_spu_RXX;
            ext_mix_word = control_registers[0x1AA / 2];
            ext_mix_word &= 0xFFFD;
        } else {
            control_registers = g_spu_RXX;
            ext_mix_word = control_registers[0x1AA / 2];
            ext_mix_word |= 2;
        }
        control_registers[0x1AA / 2] = ext_mix_word;
    }
}
