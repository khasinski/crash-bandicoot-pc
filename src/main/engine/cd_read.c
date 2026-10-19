#include "common.h"

s32 Cd_ReadSectors(s32 sector_count, s32 first_sector, void *destination)
{
    u8 position[3];
    CdIntToPos(first_sector, position);
    CdControl(2, position, NULL);
    CdRead(sector_count, destination, 0x80);
    return CdReadSync(0, NULL) == 0;
}
