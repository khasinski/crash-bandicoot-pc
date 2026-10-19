#include "common.h"
/* The retail memmove, renamed so that it does not replace the host C
 * library's. The formatter copies from its digit buffer or an input string.
 * Retail returns the advanced destination for forward copies and the
 * original destination for backward copies. Keep this asymmetric result.
 */
u8 *LIBC_Memmove(u8 *destination,const u8 *source,s32 length)
{
    if (destination >= source) {
        u8 *last;
        const u8 *source_last;
        if (length-- > 0) {
            last = destination + length;
            source_last = source + length;
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
