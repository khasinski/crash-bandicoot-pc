/*
 * --fps60: one interpolated frame between the game's 30 Hz logic frames.
 *
 * After each logic frame the camera and every object's position, rotation
 * and scale are recorded. Before drawing the next logic frame, the game
 * memory is saved, the camera and objects are moved halfway between the
 * previous and the current state, that frame is drawn and presented, and
 * the memory is restored. Drawing has side effects on game state (screen
 * shake counters, animation caches, object fields read by the logic); the
 * restore keeps them out of the simulation, which therefore runs exactly
 * as at 30 frames per second. What is kept across the restore: the render
 * buffers (the buffer swap and its timestamps belong to the presented
 * frame) and the tick counter that the root-counter interrupt advances.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "ram.h"
#include "platform.h"

struct Node;
void Gool_ForEachObject(int32_t (*visitor)(struct Node *, void *), void *context);

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
#define SNAPSHOT_SIZE 0x200000u
/* A larger step between logic frames is a teleport, not motion. */
#define MAX_STEP (1 << 20)

typedef struct ObjectState {
    uint32_t addr, exec, entity;
    int32_t trans[3], rot[3], scale[3];
} ObjectState;

typedef struct FrameState {
    int32_t camera[6];
    int count;
    ObjectState objects[MAX_OBJECTS];
} FrameState;

static int enabled = -1;
static int object_render = 1;
static int mid_frame_presented;
static int have_previous;
static FrameState previous, current;
static unsigned char *ram_copy;
static unsigned char scratch_copy[RAM_SCRATCH_SIZE];

int Platform_InterpolateFrames(void)
{
    if (enabled < 0) {
        const char *env = getenv("CRASH_FPS60");
        enabled = env && env[0] == '1';
    }
    return enabled;
}

int Platform_ObjectRender(void)
{
    return object_render;
}

void Platform_SetObjectRender(int on)
{
    object_render = on;
}

int Platform_MidFramePresented(void)
{
    int presented = mid_frame_presented;
    mid_frame_presented = 0;
    return presented;
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

static void record(FrameState *state)
{
    int i;
    for (i = 0; i < 6; i++)
        memcpy(&state->camera[i], g_cam_trans_x + 4 * i, 4);
    state->count = 0;
    Gool_ForEachObject(record_object, state);
}

static int32_t mid(int32_t a, int32_t b)
{
    return a + (int32_t)(((int64_t)b - a) / 2);
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

static const ObjectState *find_previous(const ObjectState *o)
{
    int i;
    for (i = 0; i < previous.count; i++) {
        const ObjectState *p = &previous.objects[i];
        if (p->addr == o->addr)
            return p->exec == o->exec && p->entity == o->entity ? p : NULL;
    }
    return NULL;
}

int Platform_BeginMidFrame(void)
{
    int i, k;
    int32_t camera[6];

    if (!have_previous)
        return 0;
    record(&current);
    if (ram_copy == NULL && (ram_copy = malloc(SNAPSHOT_SIZE)) == NULL)
        return 0;
    memcpy(ram_copy, g_ps1_ram, SNAPSHOT_SIZE);
    memcpy(scratch_copy, g_ps1_scratch, RAM_SCRATCH_SIZE);

    if (!far_apart(previous.camera, current.camera)) {
        for (i = 0; i < 3; i++) {
            camera[i] = mid(previous.camera[i], current.camera[i]);
            camera[3 + i] = mid_angle(previous.camera[3 + i], current.camera[3 + i]);
        }
        memcpy(g_cam_trans_x, camera, sizeof(camera));
    }
    for (k = 0; k < current.count; k++) {
        const ObjectState *o = &current.objects[k];
        const ObjectState *p = find_previous(o);
        if (p == NULL || far_apart(p->trans, o->trans))
            continue;
        for (i = 0; i < 3; i++) {
            set_word(o->addr + OBJ_TRANS + 4 * i, mid(p->trans[i], o->trans[i]));
            set_word(o->addr + OBJ_ROT + 4 * i, mid_angle(p->rot[i], o->rot[i]));
            set_word(o->addr + OBJ_SCALE + 4 * i, mid(p->scale[i], o->scale[i]));
        }
    }
    return 1;
}

void Platform_EndMidFrame(void)
{
    uint32_t keep_lo = Ram_Addr(g_context) & 0x1FFFFF;
    uint32_t keep_hi = Ram_Addr(g_ns) & 0x1FFFFF;
    uint32_t counter = Ram_Addr(frame_counter) & 0x1FFFFF;
    int32_t ticks;

    memcpy(&ticks, g_ps1_ram + counter, 4);
    memcpy(g_ps1_ram, ram_copy, keep_lo);
    memcpy(g_ps1_ram + keep_hi, ram_copy + keep_hi, SNAPSHOT_SIZE - keep_hi);
    memcpy(g_ps1_ram + counter, &ticks, 4);
    memcpy(g_ps1_scratch, scratch_copy, RAM_SCRATCH_SIZE);
    mid_frame_presented = 1;
}

void Platform_RecordFrame(void)
{
    record(&previous);
    have_previous = 1;
}
