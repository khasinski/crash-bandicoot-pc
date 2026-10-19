#include "common.h"

const char *CdComstr(s32 command)
{
    u32 index=command & 0xFF;
    const char *name;
    if(index<28)name=g_cd_com_str_ptrs[index];else name=g_str_names_none;
    return name;
}
const char *CdIntstr(s32 status)
{
    u32 index=status & 0xFF;
    const char *name;
    if(index<7)name=g_cd_intr_str_ptrs[index];else name=g_str_names_none;
    return name;
}
