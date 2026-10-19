#include "common.h"

s32 CdRead(s32 count, void *buffer, s32 mode)
{
    s32 retries = 3;
    do {
        if (cd_read_retry(buffer, count, mode) != 0) {
            retries--;
        } else {
            return 1;
        }
    } while (retries != -1);
    return 0;
}
