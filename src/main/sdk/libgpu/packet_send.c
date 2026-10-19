#include "gpu.h"

void DrawPrim(GpuPacketHeader *packet) {
    u8 length;
    length=packet->length;
    g_gpu_ctl->callback3C(0);
    g_gpu_ctl->send14(&packet->color,length);
}
