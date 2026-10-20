/*
 * The Escape menu and the settings behind it.
 *
 * Escape pauses the game inside VSync and opens a small panel drawn straight
 * into the displayed part of VRAM, so it needs no UI library and appears the
 * same on every backend. The game does not run while the menu is open; the
 * clock it reads is put back on exit so no time passes for it.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psyz.h>
#include "common.h"
#include "platform.h"

#define SETTINGS_FILE "crash-bandicoot-pc.ini"

PlatformSettings g_settings = {0, 1, 0};
int g_settings_persist;


/* The VRAM rectangle the screen currently shows. */
static int disp_x, disp_y, disp_w = 512, disp_h = 240;

void Platform_NoteDisplay(int x, int y, int w, int h)
{
    disp_x = x;
    disp_y = y;
    disp_w = w;
    disp_h = h;
}

/* --- settings file ------------------------------------------------------- */

static int clamp(int value, int low, int high)
{
    return value < low ? low : value > high ? high : value;
}

void Settings_Load(void)
{
    FILE *in = g_settings_persist ? fopen(SETTINGS_FILE, "r") : NULL;
    char line[128];
    int value;

    if (in == NULL)
        return;
    while (fgets(line, sizeof(line), in)) {
        if (sscanf(line, "fps60=%d", &value) == 1)
            g_settings.fps60 = value != 0;
        else if (sscanf(line, "scale=%d", &value) == 1)
            g_settings.scale = clamp(value, 1, PSYZ_INTERNAL_RES_MAX);
        else if (sscanf(line, "widescreen=%d", &value) == 1)
            g_settings.widescreen = value != 0;
    }
    fclose(in);
}

void Settings_Save(void)
{
    FILE *out = g_settings_persist ? fopen(SETTINGS_FILE, "w") : NULL;

    if (out == NULL)
        return;
    fprintf(out, "fps60=%d\nscale=%d\nwidescreen=%d\n", g_settings.fps60,
            g_settings.scale, g_settings.widescreen);
    fclose(out);
}

void Settings_Apply(void)
{
    Platform_SetInterpolation(g_settings.fps60);
    Psyz_VideoSetInternalResolution((unsigned)g_settings.scale);
    Platform_SetWidescreen(g_settings.widescreen);
}

/* Widescreen is a setting (the user wants it) and a state (the scene on
 * screen can show it): 3D gameplay only, so 2D screens keep their 4:3. */
static int wide_wanted, wide_shown;

void Platform_SetWidescreen(int on)
{
    wide_wanted = on;
    if (!on)
        Platform_SetWideActive(0);
}

int Platform_WideWanted(void)
{
    return wide_wanted;
}

int Platform_WideShown(void)
{
    return wide_shown;
}

void Platform_SetWideActive(int active)
{
    active = active && wide_wanted;
    if (active == wide_shown)
        return;
    wide_shown = active;
    Psyz_GteSetXScale(active ? 3 : 1, active ? 4 : 1);
    Psyz_VideoSetAspectRatio(active ? 16.0f / 9.0f : 0.0f);
}

/* --- a 5x7 font: rows of five bits, most significant bit leftmost --------- */

typedef struct Glyph {
    char ch;
    unsigned char rows[7];
} Glyph;

static const Glyph font[] = {
    {'A', {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
    {'B', {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}},
    {'C', {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}},
    {'D', {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}},
    {'E', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}},
    {'F', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}},
    {'G', {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}},
    {'H', {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
    {'I', {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}},
    {'J', {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}},
    {'K', {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}},
    {'L', {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}},
    {'M', {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}},
    {'N', {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}},
    {'O', {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {'P', {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}},
    {'Q', {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}},
    {'R', {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}},
    {'S', {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}},
    {'T', {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
    {'U', {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
    {'V', {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}},
    {'W', {0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11}},
    {'X', {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}},
    {'Y', {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}},
    {'Z', {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}},
    {'0', {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}},
    {'1', {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}},
    {'2', {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}},
    {'3', {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E}},
    {'4', {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}},
    {'5', {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}},
    {'6', {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}},
    {'7', {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}},
    {'8', {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}},
    {'9', {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}},
    {':', {0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00}},
    {'.', {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C}},
    {'/', {0x01, 0x02, 0x02, 0x04, 0x08, 0x08, 0x10}},
    {'-', {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}},
    {'>', {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08}},
};

static const unsigned char *glyph_rows(char ch)
{
    size_t i;

    for (i = 0; i < sizeof(font) / sizeof(font[0]); i++)
        if (font[i].ch == ch)
            return font[i].rows;
    return NULL;
}

/* --- drawing -------------------------------------------------------------- */

#define COLOR(r, g, b) ((u16)((r) | ((g) << 5) | ((b) << 10) | 0x8000))
#define PANEL_BACK COLOR(2, 3, 8)
#define PANEL_EDGE COLOR(31, 20, 2)
#define TEXT_NORMAL COLOR(26, 26, 28)
#define TEXT_SELECTED COLOR(31, 28, 4)

#define MENU_COLUMNS 24 /* characters per line */
#define MENU_MARGIN 10  /* font pixels left of the text */

typedef struct Panel {
    u16 *pixels;
    int w, h;       /* in VRAM pixels */
    int sx, sy;     /* VRAM pixels per font pixel */
} Panel;

static void fill(Panel *p, int x, int y, int w, int h, u16 color)
{
    int row, col;

    for (row = y; row < y + h && row < p->h; row++)
        for (col = x; col < x + w && col < p->w; col++)
            if (row >= 0 && col >= 0)
                p->pixels[row * p->w + col] = color;
}

static void put_text(Panel *p, int x, int y, const char *text, u16 color)
{
    int advance = 6 * p->sx;
    int gy, gx;

    for (; *text; text++, x += advance) {
        const unsigned char *rows = glyph_rows(*text);

        if (rows == NULL)
            continue;
        for (gy = 0; gy < 7; gy++)
            for (gx = 0; gx < 5; gx++)
                if (rows[gy] & (0x10 >> gx))
                    fill(p, x + gx * p->sx, y + gy * p->sy, p->sx, p->sy, color);
    }
}

enum { ITEM_FPS, ITEM_RESOLUTION, ITEM_WIDESCREEN, ITEM_RESUME, ITEM_QUIT, ITEM_COUNT };

static int menu_item[ITEM_COUNT];
static int menu_count;

static void build_items(void)
{
    int i;

    menu_count = 0;
    for (i = 0; i < ITEM_COUNT; i++)
        menu_item[menu_count++] = i;
}

static const char *item_label[ITEM_COUNT] = {
    "60 FPS", "RESOLUTION", "ASPECT RATIO", "RESUME", "QUIT GAME"};

static void item_value(int item, char *out, size_t size)
{
    out[0] = 0;
    switch (item) {
    case ITEM_FPS:
        snprintf(out, size, "%s", g_settings.fps60 ? "ON" : "OFF");
        break;
    case ITEM_RESOLUTION:
        snprintf(out, size, "%dX", g_settings.scale);
        break;
    case ITEM_WIDESCREEN:
        snprintf(out, size, "%s", g_settings.widescreen ? "16:9" : "4:3");
        break;
    }
}

static void line_text(char *out, size_t size, const char *label, const char *value)
{
    int pad = MENU_COLUMNS - (int)strlen(label) - (int)strlen(value);

    snprintf(out, size, "%s%*s%s", label, pad > 1 ? pad : 1, "", value);
}

/* Panel geometry in VRAM pixels, centered on what the screen shows. */
static void panel_rect(Panel *p, int *x, int *y)
{
    /* A 512 wide mode has pixels about 1.6 times taller than wide. */
    p->sx = disp_w >= 400 ? 3 : 2;
    p->sy = 2;
    p->w = (MENU_COLUMNS * 6 + 2 * MENU_MARGIN) * p->sx;
    p->h = (menu_count * 12 + 52) * p->sy;
    *x = disp_x + (disp_w - p->w) / 2;
    *y = disp_y + (disp_h - p->h) / 2;
}

static void draw_menu(int selected, u16 *pixels)
{
    Panel p;
    int x, y, i, ty;
    char value[16], line[64];

    panel_rect(&p, &x, &y);
    p.pixels = pixels;
    fill(&p, 0, 0, p.w, p.h, PANEL_EDGE);
    fill(&p, 2 * p.sx, 2 * p.sy, p.w - 4 * p.sx, p.h - 4 * p.sy, PANEL_BACK);
    put_text(&p, (p.w - 7 * 6 * p.sx) / 2, 6 * p.sy, "OPTIONS", PANEL_EDGE);
    ty = 24 * p.sy;
    for (i = 0; i < menu_count; i++, ty += 12 * p.sy) {
        item_value(menu_item[i], value, sizeof(value));
        line_text(line, sizeof(line), item_label[menu_item[i]], value);
        if (i == selected)
            put_text(&p, 3 * p.sx, ty, ">", TEXT_SELECTED);
        put_text(&p, MENU_MARGIN * p.sx, ty, line, i == selected ? TEXT_SELECTED : TEXT_NORMAL);
    }
    put_text(&p, MENU_MARGIN * p.sx, ty + 4 * p.sy, "UP/DOWN  LEFT/RIGHT  ESC", TEXT_NORMAL);
}

/* --- the menu loop --------------------------------------------------------- */

#define BUTTON_START 0x0800
#define BUTTON_UP 0x1000
#define BUTTON_RIGHT 0x2000
#define BUTTON_DOWN 0x4000
#define BUTTON_LEFT 0x8000
#define BUTTON_CIRCLE 0x0020
#define BUTTON_CROSS 0x0040

static void show(u16 *pixels, const Panel *p, int x, int y)
{
    RECT rect = {(short)x, (short)y, (short)p->w, (short)p->h};

    (LoadImage)(&rect, (u_long *)pixels);
    (DrawSync)(0);
}

static void change(int item, int direction)
{
    switch (item) {
    case ITEM_FPS:
        g_settings.fps60 = !g_settings.fps60;
        break;
    case ITEM_RESOLUTION: {
        static const int steps[] = {1, 2, 3, 4, 6, 8};
        int i, count = (int)(sizeof(steps) / sizeof(steps[0]));
        for (i = 0; i < count - 1 && steps[i] < g_settings.scale; i++)
            ;
        i = (i + direction + count) % count;
        g_settings.scale = steps[i];
        break;
    }
    case ITEM_WIDESCREEN:
        g_settings.widescreen = !g_settings.widescreen;
        break;
    }
    Settings_Apply();
}

void Platform_Menu(void (*preview)(void))
{
    Panel p;
    int x, y, selected = 0, redraw = 1;
    unsigned int previous = 0xFFFF;
    s32 ticks = frame_counter; /* the root-counter tick count */
    u16 *saved, *pixels;
    RECT rect;

    build_items();
    panel_rect(&p, &x, &y);
    saved = malloc((size_t)p.w * (size_t)p.h * sizeof(u16));
    pixels = malloc((size_t)p.w * (size_t)p.h * sizeof(u16));
    if (saved == NULL || pixels == NULL) {
        free(saved);
        free(pixels);
        return;
    }
    rect = (RECT){(short)x, (short)y, (short)p.w, (short)p.h};
    (DrawSync)(0);
    (StoreImage)(&rect, (u_long *)saved);
    (DrawSync)(0);

    for (;;) {
        unsigned int buttons, pressed;

        if (redraw) {
            draw_menu(selected, pixels);
            show(pixels, &p, x, y);
            redraw = 0;
        }
        (VSync)(0);
        Platform_MenuTick(1);
        if (preview != NULL) {
            preview();
            break;
        }
        if (Psyz_QuitRequested())
            exit(EXIT_SUCCESS);
        buttons = (unsigned int)PadRead(0);
        pressed = buttons & ~previous;
        previous = buttons;
        if (Psyz_EscapePressed() || (pressed & (BUTTON_CIRCLE | BUTTON_START)))
            break;
        if (pressed & BUTTON_UP) {
            selected = (selected + menu_count - 1) % menu_count;
            redraw = 1;
        } else if (pressed & BUTTON_DOWN) {
            selected = (selected + 1) % menu_count;
            redraw = 1;
        } else if (pressed & (BUTTON_LEFT | BUTTON_RIGHT | BUTTON_CROSS)) {
            int direction = (pressed & BUTTON_LEFT) ? -1 : 1;

            int item = menu_item[selected];

            if (item == ITEM_RESUME && (pressed & BUTTON_CROSS))
                break;
            if (item == ITEM_QUIT && (pressed & BUTTON_CROSS))
                exit(EXIT_SUCCESS);
            if (item < ITEM_RESUME) {
                change(item, direction);
                redraw = 1;
            }
        }
    }
    show(saved, &p, x, y);
    Platform_MenuTick(0);
    frame_counter = ticks;
    Settings_Save();
    if (!g_settings_persist)
        fprintf(stderr, "menu closed: fps60=%d scale=%d widescreen=%d\n", g_settings.fps60,
                g_settings.scale, g_settings.widescreen);
    free(saved);
    free(pixels);
}
