/*
 * The BIOS calls the game makes through its A0/B0 trampolines. libc-style
 * calls map to the host C library; file calls (memory card) go to PsyZ.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

s32 BIOS_AToI(const char *text)
{
    return atoi(text);
}

_Noreturn void BIOS_Abs(s32 code)
{
    fprintf(stderr, "Game called exit(%d)\n", (int)code);
    exit(code == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

void BIOS_BCopy(const void *src, void *dst, s32 size)
{
    if (size > 0)
        memmove(dst, src, (size_t)size);
}

void BIOS_BZero(void *dst, s32 size)
{
    if (size > 0)
        memset(dst, 0, (size_t)size);
}

void *BIOS_MemChr(const void *src, s32 value, s32 size)
{
    if (src == NULL || size <= 0)
        return NULL;
    return memchr(src, value, (size_t)size);
}

void *BIOS_MemCpy(void *dst, const void *src, s32 size)
{
    if (dst == NULL || size <= 0)
        return NULL;
    return memcpy(dst, src, (size_t)size);
}

void *BIOS_MemSet(void *dst, s32 value, s32 size)
{
    if (dst == NULL || size <= 0)
        return NULL;
    return memset(dst, value, (size_t)size);
}

s32 BIOS_Printf(const char *format, ...)
{
    va_list args;
    int result;
    va_start(args, format);
    result = vprintf(format, args);
    va_end(args);
    return result;
}

char *BIOS_StrCat(char *dst, const char *src)
{
    if (dst == NULL || src == NULL)
        return NULL;
    return strcat(dst, src);
}

s32 BIOS_StrCmp(const char *a, const char *b)
{
    if (a == NULL)
        return b == NULL ? 0 : -1;
    if (b == NULL)
        return 1;
    return strcmp(a, b);
}

char *BIOS_StrCpy(char *dst, const char *src)
{
    if (dst == NULL || src == NULL)
        return NULL;
    return strcpy(dst, src);
}

s32 BIOS_StrLen(const char *text)
{
    return text == NULL ? 0 : (s32)strlen(text);
}

s32 BIOS_StrNCmp(const char *a, const char *b, s32 size)
{
    if (size <= 0)
        return 0;
    return strncmp(a, b, (size_t)size);
}

s32 BIOS_Read(s32 fd, void *dst, s32 size)
{
    return (s32)read(fd, dst, size);
}

s32 BIOS_Write(s32 fd, const void *src, s32 size)
{
    return (s32)write(fd, (char *)src, size);
}

s32 BIOS_Format(const char *device)
{
    return (s32)format((char *)device);
}

void BIOS_DeliverEvent(u32 event_class, u32 spec)
{
    DeliverEvent(event_class, spec);
}
