#include "common.h"

s32 CdGetSector(s32 arg0, s32 arg1) {
    return CD_getsector(arg0, arg1) == 0;
}
