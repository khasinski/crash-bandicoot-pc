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
    amount += RAM_AT(RenderBuffer, g_ot_cur)->word78;
    RAM_AT(RenderBuffer, g_ot_cur)->word78 = amount;
}

s32 *Gpu_GetCurPrimsTail(void)
{
    return &RAM_AT(RenderBuffer, g_ot_cur)->word78;
}
