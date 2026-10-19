#include "common.h"

/* Set length and command for flat, textured-flat and Gouraud-shaded triangles. */

#include "gpu.h"

void SetPolyF3(GpuPacketHeader *packet)
{
    packet->length = 4;
    packet->command = 0x20;
}

void SetPolyFT3(GpuPacketHeader *packet)
{
    packet->length = 7;
    packet->command = 0x24;
}

void SetPolyG3(GpuPacketHeader *packet)
{
    packet->length = 6;
    packet->command = 0x30;
}

/* Additional polygon, line, sprite and tile packet headers, including polyline terminators. */

void SetPolyGT3(GpuPacketHeader *packet)
{
    packet->length=9;
    packet->command=0x34;
}

void SetPolyF4(GpuPacketHeader *packet)
{
    packet->length=5;
    packet->command=0x28;
}

void SetPolyFT4(GpuPacketHeader *packet)
{
    packet->length=9;
    packet->command=0x2C;
}

void SetPolyG4(GpuPacketHeader *packet)
{
    packet->length=8;
    packet->command=0x38;
}

void SetPolyGT4(GpuPacketHeader *packet)
{
    packet->length=12;
    packet->command=0x3C;
}

void SetSprt8(GpuPacketHeader *packet)
{
    packet->length=3;
    packet->command=0x74;
}

void SetSprt16(GpuPacketHeader *packet)
{
    packet->length=3;
    packet->command=0x7C;
}

void SetSprt(GpuPacketHeader *packet)
{
    packet->length=4;
    packet->command=0x64;
}

void SetTile1(GpuPacketHeader *packet)
{
    packet->length=2;
    packet->command=0x68;
}

void SetTile8(GpuPacketHeader *packet)
{
    packet->length=2;
    packet->command=0x70;
}

void SetTile16(GpuPacketHeader *packet)
{
    packet->length=2;
    packet->command=0x78;
}

void SetTile(GpuPacketHeader *packet)
{
    packet->length=3;
    packet->command=0x60;
}

void SetLineF2(GpuPacketHeader *packet)
{
    packet->length=3;
    packet->command=0x40;
}

void SetLineG2(GpuPacketHeader *packet)
{
    packet->length=4;
    packet->command=0x50;
}

typedef struct GpuLineF3 {
    GpuPacketHeader header;
    u32 v[3];
    u32 term;
} GpuLineF3;

typedef struct GpuLineG3 {
    GpuPacketHeader header;
    u32 v[5];
    u32 term;
} GpuLineG3;

typedef struct GpuLineF4 {
    GpuPacketHeader header;
    u32 v[4];
    u32 term;
} GpuLineF4;

typedef struct GpuLineG4 {
    GpuPacketHeader header;
    u32 v[7];
    u32 term;
} GpuLineG4;

typedef struct GpuMaskPacket {
    GpuPacketHeader header;
    u32 mask;
} GpuMaskPacket;

void SetLineF3(GpuLineF3 *packet)
{
    u32 terminator=0x55555555;
    packet->header.length=5;
    packet->header.command=0x48;
    packet->term=terminator;
}

void SetLineG3(GpuLineG3 *packet)
{
    u32 terminator=0x55555555;
    packet->header.length=7;
    packet->header.command=0x58;
    packet->term=terminator;
}

void SetLineF4(GpuLineF4 *packet)
{
    u32 terminator=0x55555555;
    packet->header.length=6;
    packet->header.command=0x4C;
    packet->term=terminator;
}

void SetLineG4(GpuLineG4 *packet)
{
    u32 terminator=0x55555555;
    packet->header.length=9;
    packet->header.command=0x5C;
    packet->term=terminator;
}

void SetBlockFill(GpuPacketHeader *packet)
{
    packet->length=3;
    packet->command=0x2;
}

void SetDrawMove(GpuMaskPacket *packet)
{
    packet->header.length=5;
    packet->header.command=0x1;
    packet->mask=0x80000000;
}
