#include "resource.h"
typedef ResourceHandle Request;
typedef ResourceEntry Entry;

s32 Ns_EIDType(Request *arg0) {
    s32 temp_v1;
    s32 var_v1;
    u32 temp_a2;

    temp_v1 = arg0->entry;
    if (!(temp_v1 & 3)) {
        return RAM_AT(Entry, temp_v1)->kind;
    }
    temp_a2 = arg0->id;
    var_v1 = 4;
    if (temp_a2 & 1) {
        gp_eid_str_term = 0;
        temp_a2 = temp_a2 >> 1;
        do {
            gp_eid_str_buf[var_v1] = g_eid_charset_table[temp_a2 & 0x3F];
            var_v1 -= 1;
            temp_a2 = temp_a2 >> 6;
        } while (var_v1 >= 0);
    } else {
        sprintf(gp_eid_str_buf, gp_str_eid_hex_fmt, temp_a2);
    }
    switch (gp_eid_type_char) {
    case 0x56:
        return 1;
    case 0x58:
        return 0x14;
    case 0x53:
        return 4;
    case 0x47:
        return 2;
    case 0x57:
        return 3;
    case 0x54:
        return 5;
    case 0x4C:
        return 6;
    case 0x5A:
        return 7;
    case 0x43:
        return 0xB;
    case 0x41:
        return 0xC;
    case 0x4D:
        return 0xD;
    case 0x49:
        return 0x10;
    case 0x4E:
        return 0xE;
    case 0x44:
        return 0xF;
    case 0x50:
        return 0x11;
    case 0x55:
        return 0x12;
    case 0x42:
        return 0x13;
    default:
        return 0;
    }
}
