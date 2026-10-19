#include "gpu.h"

typedef struct {u8 next[3]; u8 length; u32 word04, word08;} Packet;

void SetTexWindow(Packet *packet, GpuRect *rect)
{
    packet->length = 2;
    packet->word04 = get_tw(rect);
    packet->word08 = 0;
}
