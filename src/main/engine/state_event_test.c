#include "common.h"

/* Storage is owned by the containing state unit. */

void Card_ResetSoftEvent(void) {
    TestEvent(gp_edesc_s_end_io);
    TestEvent(gp_edesc_s_error);
    TestEvent(gp_edesc_s_timeout);
    TestEvent(gp_edesc_s_newdev);
}
