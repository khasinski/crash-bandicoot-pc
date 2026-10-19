#include "common.h"
/* The formatter copies from its digit buffer or an input string.
 * Retail returns the advanced destination for forward copies and the
 * original destination for backward copies. Keep this asymmetric result.
 */
u8 *memmove(u8 *destination,const u8 *source,s32 length)
{
    if (destination >= source) {
        u8 *last;
        const u8 *source_last;
        if (length-- > 0) {
            last = (u8 *)(length + (u32)destination);
            source_last = (const u8 *)(length + (u32)source);
            do {
                *last-- = *source_last--;
            } while (length-- > 0);
        }
    } else {
        if (length-- > 0) {
            do {
                *destination++ = *source++;
            } while (length-- > 0);
        }
    }
    return destination;
}
