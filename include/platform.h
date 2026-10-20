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

/* Switch --fps60 interpolation on or off at run time. */
void Platform_SetInterpolation(int enabled);

/* The user's choices, kept in crash-bandicoot-pc.ini (src/platform/menu.c). */
typedef struct PlatformSettings {
    int fps60;      /* an interpolated frame between logic frames */
    int scale;      /* internal resolution multiplier, 1 to 8 */
    int widescreen; /* 16:9 instead of 4:3 */
} PlatformSettings;
extern PlatformSettings g_settings;
extern int g_widescreen_experimental; /* --widescreen: show the unfinished option */
extern int g_settings_persist; /* read and write the ini file */
void Settings_Load(void);
void Settings_Save(void);
void Settings_Apply(void);
void Platform_SetWidescreen(int on);
int Platform_WideWanted(void);
/* Whether the scene being drawn is shown in 16:9: the GTE squeezes X by 3/4
 * and the video is stretched to match. */
void Platform_SetWideActive(int active);
/* Whether the wide view is on now; the world then adds the polygons the baked
 * 4:3 lists lack. */
int Platform_WideShown(void);

/* Escape: pause and show the options. Platform_NoteDisplay tells the menu
 * which VRAM rectangle the screen shows. */
void Platform_Menu(void (*preview)(void)); /* preview: draw once, call it, close */
void Platform_NoteDisplay(int x, int y, int w, int h);
void Platform_MenuTick(int open); /* a frame passed in the menu; 0 when it closes */

/* PadRead with the --pad-at input schedule merged into port 1. */
unsigned int Platform_PadRead(int id);
#define PadRead(id) Platform_PadRead(id)

#endif
