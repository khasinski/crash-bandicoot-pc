#include "common.h"
#include "gpu.h"

/* Prepare an image-upload packet; reject payloads outside its small inline capacity. */

void SetDrawLoad(u32 *packet,GpuRect *rect)
{
    s32 words=(rect->w*rect->h+1)/2;
    s8 length=words+4;
    if((u32)(words-1)>=11) length=0;
    packet[1]=0x01000000;
    ((u8*)packet)[3]=length;
    packet[2]=0xA0000000;
    packet[3]=*(u32*)&rect->x;
    packet[4]=*(u32*)&rect->w;
}
