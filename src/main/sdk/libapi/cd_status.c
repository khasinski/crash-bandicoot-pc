#include "common.h"

u8 CdStatus(void) {
    return g_cd_read_sectors;
}
