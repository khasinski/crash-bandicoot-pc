#ifndef CRASH_PLATFORM_H
#define CRASH_PLATFORM_H

/* Host hooks around SDK calls; src/platform/main.c. VSync counts frames for
 * the --frame-limit diagnostic boundary. */
int Platform_VSync(int mode);
#define VSync(mode) Platform_VSync(mode)

/* --fps60: render an interpolated frame between logic frames (interp.c).
 * Platform_InterpolateFrames is nonzero when enabled; ObjectRender gates
 * GOOL object drawing so the logic pass can run without drawing; MidFrame
 * brackets the interpolated render (Begin returns nonzero when it applies);
 * RecordFrame stores the finished logic state. */
int Platform_InterpolateFrames(void);
int Platform_ObjectRender(void);
void Platform_SetObjectRender(int enabled);
int Platform_BeginMidFrame(void);
void Platform_EndMidFrame(void);
void Platform_RecordFrame(void);
int Platform_MidFramePresented(void);

/* PadRead with the --pad-at input schedule merged into port 1. */
unsigned int Platform_PadRead(int id);
#define PadRead(id) Platform_PadRead(id)

#endif
