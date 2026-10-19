#include "common.h"

/* Compare the end as a 32-bit PS1 address, as in the original routine. */
u32 Card_Checksum(u8 *data, u32 size)
{
    u32 checksum = 0x12345678;
    u32 end;
    if (size != 0) {
        end = size + (u32)data;
        do {
            checksum += *data++;
            checksum = (checksum << 3) | (checksum >> 29);
        } while ((u32)data < end);
    }
    return checksum;
}
