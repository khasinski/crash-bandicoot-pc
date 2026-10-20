/*
 * --fps60: an interpolated frame during the second blank of each logic frame.
 *
 * The game runs its logic at 30 Hz: Gpu_Update waits for two blanks per
 * logic frame, and the image shown during those blanks is the one from two
 * logic frames back (k-1 while frame k+1 is presented; k comes next). So
 * during the second blank this module shows a frame halfway between k-1 and
 * k: it saves the live game memory, loads the snapshot taken at the same
 * point of the previous logic frame (state k), moves the camera and objects
 * to the midpoint of their recorded k-1 and k transforms, draws the scene
 * without logic (Core_DrawRenderPasses) into the area on screen, and puts the
 * live memory back. The logic never sees any of it and runs exactly as at 30
 * frames per second; the blanks stay where the original has them.
 *
 * Kept across the swaps: the render buffers (ordering tables, environments,
 * buffer pointers, which belong to the presentation) and the tick counter
 * that the root-counter interrupt advances.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ram.h"
#include "platform.h"

struct Node;
void Gool_ForEachObject(int32_t (*visitor)(struct Node *, void *), void *context);
void Core_DrawRenderPasses(void);
void Gpu_DrawInterpolated(void);

/* Game symbols, used only for their addresses. */
extern unsigned char g_cam_trans_x[]; /* trans x, y, z, rot x, y, z */
extern unsigned char g_context[];     /* render buffers: up to g_ns */
extern unsigned char g_ns[];
extern unsigned char frame_counter[];

/* GOOL object fields (PS1 layout). */
#define OBJ_EXEC 0x20
#define OBJ_TRANS 0x80
#define OBJ_ROT 0x8C
#define OBJ_SCALE 0x98
#define OBJ_ENTITY 0x110

#define MAX_OBJECTS 256
#define RAM_USED 0x200000u
/* A larger step between logic frames is a teleport, not motion. */
#define MAX_STEP (1 << 20)

typedef struct ObjectState {
    uint32_t addr, exec, entity;
    int32_t trans[3], rot[3], scale[3];
} ObjectState;

typedef struct FrameState {
    uint32_t level[2]; /* g_ns level number and level data: same scene */
    int32_t camera[6];
    int count;
    ObjectState objects[MAX_OBJECTS];
} FrameState;

static int enabled;
static int render_only;
static int recorded; /* logic frames recorded, up to 2 */
static FrameState older, newer;  /* k-1 and k */
static unsigned char *newer_ram; /* game memory at frame k */
static unsigned char *live_ram;
static unsigned char newer_scratch[RAM_SCRATCH_SIZE], live_scratch[RAM_SCRATCH_SIZE];

int Platform_InterpolateFrames(void)
{
    return enabled;
}

void Platform_SetInterpolation(int on)
{
    if (on && newer_ram == NULL) {
        newer_ram = malloc(RAM_USED);
        live_ram = malloc(RAM_USED);
        if (newer_ram == NULL || live_ram == NULL) {
            free(newer_ram);
            free(live_ram);
            newer_ram = live_ram = NULL;
            on = 0;
        }
    }
    enabled = on != 0;
    recorded = 0; /* the snapshots are stale after any gap */
}

int Platform_RenderOnly(void)
{
    return render_only;
}

void Platform_SetRenderOnly(int on)
{
    render_only = on;
}

static int32_t word(uint32_t addr)
{
    int32_t value;
    memcpy(&value, Ram_Ptr(addr), 4);
    return value;
}

static void set_word(uint32_t addr, int32_t value)
{
    memcpy(Ram_Ptr(addr), &value, 4);
}

static int32_t record_object(struct Node *node, void *context)
{
    FrameState *state = context;
    uint32_t addr = Ram_Addr(node);
    ObjectState *o;
    int i;

    if (state->count >= MAX_OBJECTS)
        return -0xFF;
    o = &state->objects[state->count++];
    o->addr = addr;
    o->exec = (uint32_t)word(addr + OBJ_EXEC);
    o->entity = (uint32_t)word(addr + OBJ_ENTITY);
    for (i = 0; i < 3; i++) {
        o->trans[i] = word(addr + OBJ_TRANS + 4 * i);
        o->rot[i] = word(addr + OBJ_ROT + 4 * i);
        o->scale[i] = word(addr + OBJ_SCALE + 4 * i);
    }
    return -0xFF;
}

static void level_key(uint32_t key[2])
{
    memcpy(key, g_ns + 4, 4);
    memcpy(key + 1, g_ns + 0x14, 4);
}

static void record(FrameState *state)
{
    int i;
    level_key(state->level);
    for (i = 0; i < 6; i++)
        memcpy(&state->camera[i], g_cam_trans_x + 4 * i, 4);
    state->count = 0;
    Gool_ForEachObject(record_object, state);
}

static int32_t mid(int32_t a, int32_t b)
{
    return a + (int32_t)(((int64_t)b - a) / 2);
}

/* A scale that changes sign mirrors the model (Crash turning round); halfway
 * through it would collapse to a line, so the mirror just happens. */
static int32_t mid_scale(int32_t a, int32_t b)
{
    if ((a < 0) != (b < 0))
        return b;
    return mid(a, b);
}

/* Angles are 12-bit; take the short way round. */
static int32_t mid_angle(int32_t a, int32_t b)
{
    int32_t d = (int32_t)((uint32_t)(b - a) << 20) >> 20;
    return a + d / 2;
}

static int far_apart(const int32_t *a, const int32_t *b)
{
    int i;
    for (i = 0; i < 3; i++) {
        int64_t d = (int64_t)b[i] - a[i];
        if (d > MAX_STEP || d < -MAX_STEP)
            return 1;
    }
    return 0;
}

static const ObjectState *find_older(const ObjectState *o)
{
    int i;
    for (i = 0; i < older.count; i++) {
        const ObjectState *p = &older.objects[i];
        if (p->addr == o->addr)
            return p->exec == o->exec && p->entity == o->entity ? p : NULL;
    }
    return NULL;
}

/* Copy saved game memory into the image, leaving the render buffers and the
 * tick counter as they are. */
static void swap_in(const unsigned char *ram, const unsigned char *scratch)
{
    uint32_t keep_lo = Ram_Addr(g_context) & 0x1FFFFF;
    uint32_t keep_hi = Ram_Addr(g_ns) & 0x1FFFFF;
    uint32_t counter = Ram_Addr(frame_counter) & 0x1FFFFF;
    int32_t ticks;

    memcpy(&ticks, g_ps1_ram + counter, 4);
    memcpy(g_ps1_ram, ram, keep_lo);
    memcpy(g_ps1_ram + keep_hi, ram + keep_hi, RAM_USED - keep_hi);
    memcpy(g_ps1_ram + counter, &ticks, 4);
    memcpy(g_ps1_scratch, scratch, RAM_SCRATCH_SIZE);
}

void Platform_MidFrame(int allowed)
{
    int i, k;
    uint32_t live_level[2];

    level_key(live_level);
    /* Both recorded frames and the live one must show the same loaded level;
     * across a level change the snapshot belongs to another scene. */
    if (!allowed || recorded < 2 || memcmp(older.level, newer.level, sizeof(live_level))
        || memcmp(newer.level, live_level, sizeof(live_level)))
        return;
    memcpy(live_ram, g_ps1_ram, RAM_USED);
    memcpy(live_scratch, g_ps1_scratch, RAM_SCRATCH_SIZE);
    swap_in(newer_ram, newer_scratch);

    if (!far_apart(older.camera, newer.camera)) {
        int32_t camera[6];
        for (i = 0; i < 3; i++) {
            camera[i] = mid(older.camera[i], newer.camera[i]);
            camera[3 + i] = mid_angle(older.camera[3 + i], newer.camera[3 + i]);
        }
        memcpy(g_cam_trans_x, camera, sizeof(camera));
    }
    for (k = 0; k < newer.count; k++) {
        const ObjectState *o = &newer.objects[k];
        const ObjectState *p = find_older(o);
        if (p == NULL || far_apart(p->trans, o->trans))
            continue;
        for (i = 0; i < 3; i++) {
            set_word(o->addr + OBJ_TRANS + 4 * i, mid(p->trans[i], o->trans[i]));
            set_word(o->addr + OBJ_ROT + 4 * i, mid_angle(p->rot[i], o->rot[i]));
            set_word(o->addr + OBJ_SCALE + 4 * i, mid_scale(p->scale[i], o->scale[i]));
        }
    }
    Core_DrawRenderPasses();
    Gpu_DrawInterpolated();
    swap_in(live_ram, live_scratch);
}

void Platform_RecordFrame(void)
{
    older = newer;
    record(&newer);
    memcpy(newer_ram, g_ps1_ram, RAM_USED);
    memcpy(newer_scratch, g_ps1_scratch, RAM_SCRATCH_SIZE);
    if (recorded < 2)
        recorded++;
}
