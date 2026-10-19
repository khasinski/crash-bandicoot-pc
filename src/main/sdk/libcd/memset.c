#include "common.h"
#include "gpu.h"

void memset(buf, val, count)
u8 *buf;
u8 val;
s32 count;
{
    while (--count != -1) {
        *buf++ = val;
    }
}
