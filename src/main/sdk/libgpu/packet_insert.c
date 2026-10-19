#include "gpu.h"

/* Insert destination after source, preserving both packet lengths. */
void AddPrim(GpuPacketTag *source, GpuPacketTag *destination)
{
    destination->next = source->next;
    source->next = (u32)destination;
}

/* Copy source's successor to destination, then replace source's link. */
void AddPrims(GpuPacketTag *source, u32 address, GpuPacketTag *destination)
{
    destination->next = source->next;
    source->next = address;
}
