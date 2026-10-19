#include "common.h"

/* Format an encoded resource ID, or encode five characters with the 64-character alphabet. */

char *Ns_EIDValToString(s32 id)
{
    if (id & 1) {
        sprintf(gp_page_id_str_buf, gp_str_page_id_fmt, id >> 1);
    } else {
        sprintf(gp_page_id_str_buf, gp_str_eid_hex_fmt, id);
    }
    return gp_page_id_str_buf;
}

s32 Ns_StringToEID(u8 *name)
{
    s32 id = 0;
    u8 *end = name + 5;
    s32 index;

    do {
        id <<= 6;
        for (index = 0; index < 0x40; index++) {
            if (g_eid_charset_table[index] == *name) {
                id |= index;
                break;
            }
        }
        name++;
    } while (name < end);
    id <<= 1;
    return id | 1;
}
