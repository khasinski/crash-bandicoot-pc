#ifndef CRASH_PLATFORM_H
#define CRASH_PLATFORM_H

/* Host hooks around SDK calls; src/platform/main.c. VSync counts frames for
 * the --frame-limit diagnostic boundary. */
int Platform_VSync(int mode);
#define VSync(mode) Platform_VSync(mode)

#endif
