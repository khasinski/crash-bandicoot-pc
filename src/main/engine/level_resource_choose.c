#include "resource.h"
#include "ns.h"

void Pbak_Choose(s32 *resource_id)
{
    u8 name[8];
    s32 count = Ns_FileOpen(&g_ns, 19);

    __builtin_memcpy(name, gp_str_pbak_name_tmpl, 6);
    BIOS_MemSet(name + 6, 0, 2);
    if (count == 0) {
        s32 mode = gp_pbak_mode;

        *resource_id = 0x6396347F;
        if (mode != 25)
            g_game_substate = 0;
    } else {
        name[2] = Math_Randb(count) + 0x30;
        name[3] = Ns_AlphaNumChar(g_ns_cur_level_id[0]);
        *resource_id = Ns_StringToEID(name);
        g_ns_load_done_flag[0] = -1;
        g_pbak_frame_step = 3;
    }
}
