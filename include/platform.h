#ifndef CRASH_PLATFORM_H
#define CRASH_PLATFORM_H

/* Host hooks around SDK calls; src/platform/main.c. VSync counts frames for
 * the --frame-limit diagnostic boundary. */
int Platform_VSync(int mode);
#define VSync(mode) Platform_VSync(mode)

/* --fps60: an interpolated frame between logic frames (interp.c).
 * Platform_InterpolateFrames is nonzero when enabled; Gpu_Update calls
 * MidFrame between its two blanks and RecordFrame after them. */
int Platform_InterpolateFrames(void);
void Platform_MidFrame(int allowed);
/* Nonzero during a pass that only draws objects: no input, no logic. */
int Platform_RenderOnly(void);
void Platform_SetRenderOnly(int enabled);
void Platform_RecordFrame(void);

/* PadRead with the --pad-at input schedule merged into port 1. */
unsigned int Platform_PadRead(int id);
#define PadRead(id) Platform_PadRead(id)

#endif
