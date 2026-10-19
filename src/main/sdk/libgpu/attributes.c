#include "common.h"
#include "gpu.h"

/* Set or clear the transparency and raw-texture command bits. */

void SetSemiTrans(GpuPacketHeader *packet,s32 enabled)
{
    u8 command;
    if(enabled) command=packet->command|2;
    else command=packet->command&0xFD;
    packet->command=command;
}

void SetShadeTex(GpuPacketHeader *packet,s32 enabled)
{
    u8 command;
    if(enabled) command=packet->command|1;
    else command=packet->command&0xFE;
    packet->command=command;
}
