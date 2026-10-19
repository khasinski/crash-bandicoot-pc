#include "gpu.h"

typedef struct {u8 next[3]; u8 length; u32 word04, word08;} Packet;

void SetDrawOffset(Packet *packet, s16 *offset)
{
    packet->length = 2;
    packet->word04 = get_ofs(offset[0], offset[1]);
    packet->word08 = 0;
}
