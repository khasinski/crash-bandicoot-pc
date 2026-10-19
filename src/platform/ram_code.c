#include <stdio.h>
#include <stdlib.h>
#include "ram.h"

/* Every game function whose PS1 address may be stored in game memory. The
 * declarations only provide addresses; calls go through typed pointers. */
#define RAM_CODE(addr, name) extern void name(void);
#include "../main/ram_code.inc"
#undef RAM_CODE

typedef struct RamCode {
    uint32_t addr;
    void (*fn)(void);
} RamCode;

static const RamCode ram_code[] = {
#define RAM_CODE(addr, name) {addr, name},
#include "../main/ram_code.inc"
#undef RAM_CODE
};

void *Ram_Fn(uint32_t addr)
{
    size_t lo = 0, hi = sizeof(ram_code) / sizeof(ram_code[0]);
    if (addr == 0)
        return NULL;
    addr |= 0x80000000u;
    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        if (ram_code[mid].addr < addr)
            lo = mid + 1;
        else
            hi = mid;
    }
    if (lo < sizeof(ram_code) / sizeof(ram_code[0]) && ram_code[lo].addr == addr)
        return (void *)ram_code[lo].fn;
    fprintf(stderr, "No native function for PS1 code address 0x%08X\n", addr);
    abort();
}

uint32_t Ram_FnAddr(const void *fn)
{
    size_t i;
    if (fn == NULL)
        return 0;
    for (i = 0; i < sizeof(ram_code) / sizeof(ram_code[0]); i++) {
        if ((const void *)ram_code[i].fn == fn)
            return ram_code[i].addr;
    }
    fprintf(stderr, "Function %p has no PS1 code address\n", fn);
    abort();
}
