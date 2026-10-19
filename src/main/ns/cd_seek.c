#include "common.h"

#include "ns.h"
void Ns_CdSeek(s32 index)
{
    CdlLOC position;
    if (gp_pause_B_pressed != 0 && g_ns_active_cd_slot[0] == 0 && CdSync(1, NULL) == 0) {
        CdIntToPos((u32)g_file_map[index].directory_sector & 0x7FFFFF, &position);
        CdControlF(0x15, (u_char *)&position);
    }
}
