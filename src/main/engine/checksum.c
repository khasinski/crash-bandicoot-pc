#include "common.h"

u32 Card_Checksum(u8 *data, u32 size)
{
    u32 checksum = 0x12345678;
    u8 *end;
    if (size != 0) {
        end = data + size;
        do {
            checksum += *data++;
            checksum = (checksum << 3) | (checksum >> 29);
        } while (data < end);
    }
    return checksum;
}
