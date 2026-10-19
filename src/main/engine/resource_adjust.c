#include "common.h"

/* Invalid handles are ignored; nonpositive counts use the release path. */
s32 Midi_PreloadEntries(s32 *resource, s32 argument, s32 count)
{
    s32 result;

    if (*resource == 0x6396347F) {
        result = 0;
    } else if (count <= 0) {
        Ns_Close(resource, -count);
        result = 0;
    } else {
        result = Ns_Open(resource, argument, count);
    }
    return result;
}
