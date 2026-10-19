#include "common.h"
typedef struct {u8 bytes[4];} BufferToken;
#define CRASH_EXT_ENGINE_STATE_WORD_COPY
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_STATE_WORD_COPY
void *Gpu_RoundTicks(void) {
BufferToken *base=g_pbak_frame_cursor;
 (void)frame_counter;
 *(base-2)=g_pbak_seed_val; return base-2;
}
