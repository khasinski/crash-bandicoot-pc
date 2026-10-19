#include "gpu.h"

typedef struct {u8 next[3];u8 length;u32 word04,word08;} Packet;
void SetDrawMode(Packet *packet,s32 arg1,s32 arg2,s32 arg3,GpuRect *arg4) {
    packet->length=2;
    packet->word04=get_mode(arg1,arg2,arg3&0xFFFF);
    packet->word08=get_tw(arg4);
}
