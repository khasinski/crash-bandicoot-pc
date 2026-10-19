#include "common.h"

s32 Slst_Kill(void) {
 LIBC_Free(gp_slst_poly_list_a);
 LIBC_Free(gp_slst_poly_list_b);
 return -255;
}
