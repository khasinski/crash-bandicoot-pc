#include "common.h"

/* Reset the buffer cursor and initialise its ordering table; expose the cursor word. */

#include "render.h"

void Gpu_ResetPrims(RenderBuffer *buffer)
{
    buffer->word78 = buffer->word74;
    Gpu_RResetOT(buffer->ordering_table, 0x800);
}

void Gpu_GetPrimsTail(s32 amount)
{
    amount += g_ot_cur->word78;
    g_ot_cur->word78 = amount;
}

s32 *Gpu_GetCurPrimsTail(void)
{
    return &g_ot_cur->word78;
}
