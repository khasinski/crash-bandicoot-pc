#include "common.h"

/* Set the wait flag and service pending resource operations until the counter reaches zero. */

void Ns_Update2(void)
{
    g_ns_update_lock = 1;
    while (g_ns_pending_io_cnt != 0)
        Ns_Update(-1);
}
