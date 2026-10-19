#include "common.h"

void Slst_NoopWrapper(void *a, void *b, void *c) {
    Slst_Noop2(a, b, c, 0x808080);
}
