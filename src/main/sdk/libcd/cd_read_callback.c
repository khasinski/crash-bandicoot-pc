#include "common.h"

s32 CdReadCallback(s32 arg0) {
    return CD_datasync(arg0);
}
