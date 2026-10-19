#include "common.h"

/* Pass the object and its field at 0x80 to the transform routine. */

void Zone_QueryOctrees2(u8 *object, struct SubWorkspace *ws)
{
    Zone_QueryOctrees((struct ZqVec *)(object + 0x80), (struct SubObj *)object, ws);
}
