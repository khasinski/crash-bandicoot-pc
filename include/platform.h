#ifndef CRASH_PLATFORM_H
#define CRASH_PLATFORM_H

/* Host hooks around SDK calls; src/platform/main.c. VSync counts frames for
 * the --frame-limit diagnostic boundary. */
int Platform_VSync(int mode);
#define VSync(mode) Platform_VSync(mode)

/* PadRead with the --pad-at input schedule merged into port 1. */
unsigned int Platform_PadRead(int id);
#define PadRead(id) Platform_PadRead(id)

#endif
