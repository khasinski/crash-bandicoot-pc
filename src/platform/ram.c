#include <stdio.h>
#include <stdlib.h>
#include "ram.h"

/* Diagnostics for conversions that leave game memory. */
void Ram_BadAddress(uint32_t addr)
{
    fprintf(stderr, "Invalid PS1 address 0x%08X\n", addr);
    abort();
}

void Ram_BadPointer(const void *ptr)
{
    fprintf(stderr, "Pointer %p is outside game memory\n", ptr);
    abort();
}
