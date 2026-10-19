#ifndef CRASH_RAM_H
#define CRASH_RAM_H

#include <stdint.h>

/*
 * The game's data lives in a host block that mirrors PS1 RAM: every global
 * is a label at its original address (src/main/ram_layout.S), and the block
 * is filled from SCUS_949.00 at startup. Words in game memory that hold
 * pointers keep PS1 addresses (0x80xxxxxx, or 0x1F800xxx for the
 * scratchpad). Host code converts them explicitly with Ram_Ptr/Ram_Addr.
 */
#define RAM_PS1_BASE 0x80000000u
#define RAM_SIZE 0x800000u
#define RAM_SCRATCH_BASE 0x1F800000u
#define RAM_SCRATCH_SIZE 0x400u

extern unsigned char g_ps1_ram[];
extern unsigned char g_ps1_scratch[];

void Ram_BadAddress(uint32_t addr);
void Ram_BadPointer(const void *ptr);

/* PS1 address (any KUSEG/KSEG0/KSEG1 mirror, or scratchpad) to host pointer. */
static inline void *Ram_Ptr(uint32_t addr)
{
    uint32_t phys = addr & 0x1FFFFFFFu;
    if (addr == 0)
        return NULL;
    if (phys < RAM_SIZE)
        return g_ps1_ram + phys;
    if (phys - (RAM_SCRATCH_BASE & 0x1FFFFFFFu) < RAM_SCRATCH_SIZE)
        return g_ps1_scratch + (phys - (RAM_SCRATCH_BASE & 0x1FFFFFFFu));
    Ram_BadAddress(addr);
    return NULL;
}

/* Host pointer into game memory to its KSEG0 (or scratchpad) PS1 address. */
static inline uint32_t Ram_Addr(const void *ptr)
{
    const unsigned char *p = (const unsigned char *)ptr;
    if (p == NULL)
        return 0;
    if ((uintptr_t)(p - g_ps1_ram) < RAM_SIZE)
        return RAM_PS1_BASE + (uint32_t)(p - g_ps1_ram);
    if ((uintptr_t)(p - g_ps1_scratch) < RAM_SCRATCH_SIZE)
        return RAM_SCRATCH_BASE + (uint32_t)(p - g_ps1_scratch);
    Ram_BadPointer(ptr);
    return 0;
}

/* A word in game memory holding the PS1 address of a TYPE. */
#define RAM_REF(type) uint32_t

/* Typed view of a PS1 address. */
#define RAM_AT(type, addr) ((type *)Ram_Ptr((uint32_t)(addr)))

/* Code addresses in game memory: the native function at a PS1 address, and
 * the PS1 address of a native game function (src/main/ram_code.inc). */
void *Ram_Fn(uint32_t addr);
uint32_t Ram_FnAddr(const void *fn);
#define RAM_FN(type, addr) ((type)Ram_Fn((uint32_t)(addr)))

#endif
