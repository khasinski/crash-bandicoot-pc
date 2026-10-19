/*
 * Native startup: the role of the PS1 BIOS loader and crt0. Mounts the
 * user's disc image, loads SCUS_949.00 into the RAM image, sets up the heap
 * the way crt0 does and enters the original CORE_Main.
 */
#define _XOPEN_SOURCE 700
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <SDL3/SDL.h>
#include <psyz.h>
#include "common.h"
#include "platform.h"

#define EXE_HEADER_SIZE 0x800
#define EXE_SIZE 0x47000u /* SCUS_949.00 including its header */
#define EXE_CRC32 0x6500418Au
#define SECTOR_SIZE 0x800

/* crt0 symbols: BSS starts at gp_AllocRestBlockTop and ends at g_bss_end;
 * gp_ram_size_bytes and gp_paused hold the RAM and stack sizes. */
extern u8 g_bss_end[];
extern s32 gp_ram_size_bytes;
extern s32 gp_paused;

s32 CORE_Main(void);

/* The game runs on a stack inside the RAM image, above the 2 MiB the game
 * uses, so the addresses of its locals are PS1 addresses like everything
 * else it passes to the original routines. */
#define GAME_STACK_BASE 0x80400000u
#define GAME_STACK_SIZE 0x3C0000u

static ucontext_t host_context, game_context;
static s32 game_result;

static u32 frame_limit;
static int headless;

/* --pad-at FRAME:MASK holds PS1 button bits (pressed = 1) from FRAME on. */
typedef struct PadEvent {
    u32 frame;
    u32 buttons;
} PadEvent;
static PadEvent *pad_events;
static int pad_event_count;
static Uint64 next_frame_ns;
static u32 frame_count;
static const char *vram_path;

/* crt0's __main runs global constructors; the game has none. */
void __main(void)
{
}

/* libgte ReadSZfifo3: the three screen Z FIFO entries (GTE data 17..19). */
void ReadSZfifo3(int *sz0, int *sz1, int *sz2)
{
    *sz0 = (int)Psyz_GteDataRead(17);
    *sz1 = (int)Psyz_GteDataRead(18);
    *sz2 = (int)Psyz_GteDataRead(19);
}

static u32 crc32(const u8 *data, size_t size)
{
    u32 crc = 0xFFFFFFFFu;
    size_t i;
    int bit;
    for (i = 0; i < size; i++) {
        crc ^= data[i];
        for (bit = 0; bit < 8; bit++)
            crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1)));
    }
    return ~crc;
}

static int load_executable(void)
{
    CdlFILE file;
    u8 *image;
    u32 load_addr, size;
    int sectors = (EXE_SIZE + SECTOR_SIZE - 1) / SECTOR_SIZE;

    if (!CdInit()) {
        fprintf(stderr, "Cannot initialize the CD image\n");
        return -1;
    }
    if (CdSearchFile(&file, "\\SCUS_949.00;1") == NULL || file.size != EXE_SIZE) {
        fprintf(stderr, "The disc is not Crash Bandicoot (USA): no SCUS_949.00\n");
        return -1;
    }
    image = malloc((size_t)sectors * SECTOR_SIZE);
    if (image == NULL)
        return -1;
    CdControl(CdlSetloc, (u_char *)&file.pos, NULL);
    if (!CdRead(sectors, (u_long *)image, CdlModeSpeed) || CdReadSync(0, NULL) != 0) {
        fprintf(stderr, "Cannot read SCUS_949.00 from the disc image\n");
        free(image);
        return -1;
    }
    if (crc32(image, EXE_SIZE) != EXE_CRC32 || memcmp(image, "PS-X EXE", 8) != 0) {
        fprintf(stderr, "SCUS_949.00 does not match the USA release\n");
        free(image);
        return -1;
    }
    memcpy(&load_addr, image + 0x18, 4);
    memcpy(&size, image + 0x1C, 4);
    memcpy(Ram_Ptr(load_addr), image + EXE_HEADER_SIZE, size);
    free(image);
    return 0;
}

/* crt0: clear BSS, give the heap the RAM between BSS and the stack, start. */
static s32 start_game(void)
{
    u32 bss_end = Ram_Addr(g_bss_end) & 0x1FFFFFFFu;
    u32 stack_top = (u32)gp_ram_size_bytes - 8;
    u32 heap_size = stack_top - (u32)gp_paused - bss_end;
    u8 *bss = (u8 *)&gp_AllocRestBlockTop;

    memset(bss, 0, (size_t)(g_bss_end - bss));
    InitHeap(Ram_Ptr((bss_end | RAM_PS1_BASE) + 4), heap_size);
    return CORE_Main();
}

/* ucontext is deprecated on macOS but still supported, and it keeps the
 * game on the main thread, which SDL requires there. */
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

static void run_game(void)
{
    game_result = start_game();
}

static void dump_vram(const char *path)
{
    static u16 vram[1024 * 512];
    RECT rect = {0, 0, 1024, 512};
    FILE *out;

    DrawSync(0);
    (StoreImage)(&rect, (u_long *)vram);
    DrawSync(0);
    out = fopen(path, "wb");
    if (out == NULL || fwrite(vram, sizeof(vram), 1, out) != 1)
        fprintf(stderr, "Cannot write VRAM to %s\n", path);
    if (out != NULL)
        fclose(out);
}

/* NTSC field rate. Headless PsyZ does not pace VSync, but the root counters
 * the game times itself with run on the wall clock, so pace it here. */
#define FIELD_NS 16683350u

int Platform_VSync(int mode)
{
    int result = (VSync)(mode); /* PsyZ, not the macro */
    if (mode == 0 && headless) {
        Uint64 now = SDL_GetTicksNS();
        if (next_frame_ns == 0 || now > next_frame_ns + 4 * (Uint64)FIELD_NS)
            next_frame_ns = now;
        next_frame_ns += FIELD_NS;
        if (next_frame_ns > now)
            SDL_DelayPrecise(next_frame_ns - now);
    }
    if (mode == 0)
        frame_count++;
    if (mode == 0 && frame_limit != 0 && frame_count >= frame_limit) {
        if (vram_path != NULL)
            dump_vram(vram_path);
        printf("{\"frames\": %u}\n", frame_count);
        exit(EXIT_SUCCESS);
    }
    return result;
}

unsigned int Platform_PadRead(int id)
{
    unsigned int buttons = (unsigned int)(PadRead)(id); /* PsyZ */
    u32 scheduled = 0;
    int i;
    for (i = 0; i < pad_event_count && pad_events[i].frame <= frame_count; i++)
        scheduled = pad_events[i].buttons;
    return buttons | scheduled;
}

static int add_pad_event(const char *text)
{
    char *end;
    unsigned long frame, buttons;
    PadEvent *grown;

    errno = 0;
    frame = strtoul(text, &end, 10);
    if (errno || *end != ':' || frame > UINT_MAX)
        return 0;
    buttons = strtoul(end + 1, &end, 0);
    if (errno || *end || buttons > 0xFFFF)
        return 0;
    if (pad_event_count && frame < pad_events[pad_event_count - 1].frame)
        return 0;
    grown = realloc(pad_events, (size_t)(pad_event_count + 1) * sizeof(*pad_events));
    if (grown == NULL)
        return 0;
    pad_events = grown;
    pad_events[pad_event_count].frame = (u32)frame;
    pad_events[pad_event_count].buttons = (u32)buttons;
    pad_event_count++;
    return 1;
}

static void usage(const char *program)
{
    printf("Usage: %s --disc <Crash Bandicoot (USA).cue> [--headless]"
           " [--frame-limit N] [--dump-vram PATH] [--pad-at FRAME:BUTTONS ...]\n", program);
    printf("--pad-at holds the PS1 button mask (pressed = 1, START = 0x800) from FRAME on.\n");
}

int main(int argc, char **argv)
{
    const char *disc = NULL;
    int i;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--help")) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (!strcmp(argv[i], "--disc") && i + 1 < argc) {
            disc = argv[++i];
        } else if (!strcmp(argv[i], "--headless")) {
            SDL_SetEnvironmentVariable(SDL_GetEnvironment(), "PSYZ_HEADLESS", "1", true);
            headless = 1;
        } else if (!strcmp(argv[i], "--frame-limit") && i + 1 < argc) {
            char *end;
            unsigned long limit;
            errno = 0;
            limit = strtoul(argv[++i], &end, 10);
            if (errno || *end || limit == 0 || limit > UINT_MAX) {
                fprintf(stderr, "Invalid frame limit\n");
                return EXIT_FAILURE;
            }
            frame_limit = (u32)limit;
        } else if (!strcmp(argv[i], "--pad-at") && i + 1 < argc) {
            if (!add_pad_event(argv[++i])) {
                fprintf(stderr, "Invalid or out-of-order --pad-at %s\n", argv[i]);
                return EXIT_FAILURE;
            }
        } else if (!strcmp(argv[i], "--dump-vram") && i + 1 < argc) {
            vram_path = argv[++i];
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (disc == NULL) {
        fprintf(stderr, "Supply --disc with the USA CUE/BIN image\n");
        return EXIT_FAILURE;
    }
    if (vram_path != NULL && frame_limit == 0) {
        fprintf(stderr, "--dump-vram requires --frame-limit\n");
        return EXIT_FAILURE;
    }
    Psyz_SetTitle("Crash Bandicoot");
    if (Psyz_CdSetDiskPath(disc) != 0) {
        fprintf(stderr, "Cannot open disc image: %s\n", disc);
        return EXIT_FAILURE;
    }
    if (load_executable() != 0)
        return EXIT_FAILURE;
    getcontext(&game_context);
    game_context.uc_stack.ss_sp = Ram_Ptr(GAME_STACK_BASE);
    game_context.uc_stack.ss_size = GAME_STACK_SIZE;
    game_context.uc_link = &host_context;
    makecontext(&game_context, run_game, 0);
    if (swapcontext(&host_context, &game_context) != 0) {
        fprintf(stderr, "Cannot switch to the game stack\n");
        return EXIT_FAILURE;
    }
    return game_result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
