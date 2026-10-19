#include "common.h"

/* Character lookup and decoding for the 64-character resource-name alphabet. */

u8 Ns_AlphaNumChar(s32 index)
{
    return g_eid_charset_table[index];
}

s32 Ns_AlphaNumIndex(s32 ch)
{
    s32 index;
    for (index = 0; index < 0x40; index++) {
        if (g_eid_charset_table[index] == (ch & 0xFF))
            return index;
    }
    if ((ch & 0xFF) == 0x2D)
        return 0x63;
    return -0xE;
}

char *Ns_GetFilename(s32 a, u32 b)
{
    return Ns_GetFilename2(a, b, 0, 0);
}

char *Ns_GetFilename2(s32 type, u32 id, s32 arg2, s32 arg3)
{
    switch (type) {
    case 0:
        if (gp_pause_B_pressed != 0) {
            sprintf(gp_nsd_filename_buf, g_str_nsd_path_fmt, id >> 4, id);
        } else {
            sprintf(gp_nsd_filename_buf, g_str_encoding_s_ss_07x_nsd, g_str_encoding_c_src_willie_target, g_str_encoding_streams, id);
        }
        break;
    case 1:
        if (gp_pause_B_pressed != 0) {
            sprintf(gp_nsd_filename_buf, g_str_encoding_s_x_s_07x, id >> 4, id);
        } else {
            sprintf(gp_nsd_filename_buf, g_str_encoding_s_ss_07x_nsf, g_str_encoding_c_src_willie_target, g_str_encoding_streams, id);
        }
        break;
    default:
        return (char *)-0xE;
    }
    return gp_nsd_filename_buf;
}
