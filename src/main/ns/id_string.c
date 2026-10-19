#include "common.h"

u8 *Ns_EIDToString(u32 id)
{
    s32 index;
    if (id & 1) {
        gp_eid_str_term = 0;
        id >>= 1;
        index = 4;
        do {
            gp_eid_str_buf[index] = g_eid_charset_table[id & 0x3F];
            index--;
            id >>= 6;
        } while (index >= 0);
    } else {
        sprintf(gp_eid_str_buf, gp_str_eid_hex_fmt, id);
    }
    return gp_eid_str_buf;
}
