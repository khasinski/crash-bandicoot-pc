#include "common.h"

/* Decode a texture-page value for the diagnostic callback. */

void DumpTPage(s32 page) {
    u32 default_page;
    u32 mode_page;

    if ((GetGraphType() == 1) || (GetGraphType() == 2)) {
        mode_page = page & 0xFFFF;
        g_gpu_ot_status(g_str_environment_print_tpage_d_d_d, (mode_page >> 9) & 3, (mode_page >> 7) & 3, (mode_page << 6) & 0x7C0, (mode_page * 8) & 0x300);
        return;
    }
    default_page = page & 0xFFFF;
    g_gpu_ot_status(g_str_environment_print_tpage_d_d_d, (default_page >> 7) & 3, (default_page >> 5) & 3, (default_page << 6) & 0x7C0, ((default_page * 0x10) & 0x100) + ((default_page >> 2) & 0x200));
}
