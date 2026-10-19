#include "common.h"

s32 Cd_ReadSectors(s32 sector_count, s32 first_sector, void *destination)
{
    CdlLOC position;
    CdIntToPos(first_sector, &position);
    CdControl(2, (u_char *)&position, NULL);
    CdRead(sector_count, (u_long *)destination, 0x80);
    return CdReadSync(0, NULL) == 0;
}
