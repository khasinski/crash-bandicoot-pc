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
#include "ns.h"

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
static u32 frame_count;
static const char *vram_path;
static const char *capture_path;
static int cli_fps60 = -1; /* -1: not given on the command line */
static unsigned cli_scale;
static int headless;

/* --menu-capture FRAME:PATH draws the Escape menu once at FRAME, captures the
 * screen to PATH and carries on, for checking how it looks. */
/* --menu-at FRAME opens the menu at FRAME; while it is open the --pad-at
 * schedule keeps advancing, one step per presented frame, to drive it. */
/* --dump-audio PATH writes the sound as a 44.1 kHz stereo WAV, pulled from
 * the SPU once per presented frame so that it follows game time. */
static const char *audio_path;
static FILE *audio_file;
static u32 audio_frames;
static double audio_due;
static u32 menu_at;
static u32 menu_ticks;
static u32 menu_capture_frame;
static const char *menu_capture_path;

/* --capture-seq DIR:FIRST:LAST writes every presented frame in the range as
 * DIR/fNNNNN.ppm, for finding flicker. */
static const char *seq_dir;
static u32 seq_first, seq_last;

/* --pad-at FRAME:MASK holds PS1 button bits (pressed = 1) from FRAME on. */
typedef struct PadEvent {
    u32 frame;
    u32 buttons;
} PadEvent;
static PadEvent *pad_events;
static int pad_event_count;

/* --warp FRAME:LEVEL requests a level change the way the map does, for
 * loading tests. */
static u32 warp_frame;
static s32 warp_level = -1;

/* --trace N prints the player's position and the game state every N frames. */
static u32 trace_period;

/* --control-at N: from frame N, read commands from stdin between frames:
 * "advance FRAMES BUTTONS", "dump PATH", "quit". Each advance ends with a
 * state line. Meant for driving the game from scripts. */
static u32 control_at;
static u32 control_until;
static u32 control_buttons;

extern u32 gp_crash_obj;

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

/* The frame as the video backend presented it, as a binary PPM. */
static void capture_frame(const char *path)
{
    int w, h;
    unsigned char *rgb = Psyz_VideoAllocCapturedFrame(&w, &h);
    FILE *out = rgb ? fopen(path, "wb") : NULL;
    if (out != NULL) {
        fprintf(out, "P6\n%d %d\n255\n", w, h);
        fwrite(rgb, 3, (size_t)w * (size_t)h, out);
        fclose(out);
    } else {
        fprintf(stderr, "Cannot capture the presented frame to %s\n", path);
    }
    free(rgb);
}

static void capture_sequence_frame(void)
{
    char path[1024];
    snprintf(path, sizeof(path), "%s/f%05u.ppm", seq_dir, frame_count);
    capture_frame(path);
}

static void menu_capture(void)
{
    capture_frame(menu_capture_path);
}

static void audio_pull(void)
{
    short buf[2 * 2048];
    int want;

    audio_due += 44100.0 / 59.94;
    want = (int)audio_due;
    audio_due -= want;
    while (want > 0) {
        int n = want > 2048 ? 2048 : want;
        Psyz_SpuPullSamples(buf, n);
        fwrite(buf, 4, (size_t)n, audio_file);
        audio_frames += (u32)n;
        want -= n;
    }
}

static void audio_finish(void)
{
    unsigned char header[44];
    u32 bytes = audio_frames * 4, riff = 36 + bytes, rate = 44100, byte_rate = 44100 * 4;
    u16 fmt = 1, channels = 2, align = 4, bits = 16;
    u32 sixteen = 16;

    memcpy(header, "RIFF", 4);
    memcpy(header + 4, &riff, 4);
    memcpy(header + 8, "WAVEfmt ", 8);
    memcpy(header + 16, &sixteen, 4);
    memcpy(header + 20, &fmt, 2);
    memcpy(header + 22, &channels, 2);
    memcpy(header + 24, &rate, 4);
    memcpy(header + 28, &byte_rate, 4);
    memcpy(header + 32, &align, 2);
    memcpy(header + 34, &bits, 2);
    memcpy(header + 36, "data", 4);
    memcpy(header + 40, &bytes, 4);
    fseek(audio_file, 0, SEEK_SET);
    fwrite(header, 1, sizeof(header), audio_file);
    fclose(audio_file);
}

static void print_state(void)
{
    const s32 *obj = gp_crash_obj ? RAM_AT(s32, gp_crash_obj) : NULL;
    printf("{\"frame\": %u, \"level\": %d, \"substate\": %d", frame_count,
           (int)g_ns.level, (int)g_game_substate);
    if (obj)
        printf(", \"x\": %d, \"y\": %d, \"z\": %d, \"status_b\": %u",
               (int)obj[0x80 / 4], (int)obj[0x84 / 4], (int)obj[0x88 / 4],
               (unsigned)obj[0xCC / 4]);
    printf("}\n");
    fflush(stdout);
}

static void run_control(void)
{
    char line[512];
    print_state();
    while (fgets(line, sizeof(line), stdin)) {
        unsigned frames, buttons;
        char path[480];
        if (sscanf(line, "advance %u %i", &frames, (int *)&buttons) == 2) {
            control_until = frame_count + frames;
            control_buttons = buttons;
            return;
        }
        if (sscanf(line, "dump %479s", path) == 1) {
            dump_vram(path);
            printf("{\"dumped\": \"%s\"}\n", path);
            fflush(stdout);
            continue;
        }
        if (!strncmp(line, "quit", 4))
            break;
    }
    exit(EXIT_SUCCESS);
}

int Platform_VSync(int mode)
{
    int result = (VSync)(mode); /* PsyZ, not the macro */
    /* Escape or closing the window: the game loop itself never ends. */
    if (Psyz_QuitRequested())
        exit(EXIT_SUCCESS);
    if (mode == 0 && Psyz_EscapePressed())
        Platform_Menu(NULL);
    if (mode == 0)
        frame_count++;
    if (mode == 0 && audio_file != NULL)
        audio_pull();
    if (mode == 0 && menu_at != 0 && frame_count == menu_at)
        Platform_Menu(NULL);
    if (mode == 0 && menu_capture_path != NULL && frame_count == menu_capture_frame)
        Platform_Menu(menu_capture);
    if (mode == 0 && seq_dir != NULL && frame_count >= seq_first && frame_count <= seq_last)
        capture_sequence_frame();
    if (mode == 0 && trace_period && frame_count % trace_period == 0)
        print_state();
    if (mode == 0 && control_at && frame_count >= control_at && frame_count >= control_until)
        run_control();
    if (mode == 0 && warp_level >= 0 && frame_count == warp_frame)
        gp_pbak_frame_idx = warp_level;
    if (mode == 0 && frame_limit != 0 && frame_count >= frame_limit) {
        if (vram_path != NULL)
            dump_vram(vram_path);
        if (capture_path != NULL)
            capture_frame(capture_path);
        if (audio_file != NULL)
            audio_finish();
        printf("{\"frames\": %u}\n", frame_count);
        exit(EXIT_SUCCESS);
    }
    return result;
}

void Platform_MenuTick(int open)
{
    menu_ticks = open ? menu_ticks + 1 : 0;
}

unsigned int Platform_PadRead(int id)
{
    unsigned int buttons = (unsigned int)(PadRead)(id); /* PsyZ */
    u32 scheduled = 0;
    int i;
    for (i = 0; i < pad_event_count && pad_events[i].frame <= frame_count + menu_ticks; i++)
        scheduled = pad_events[i].buttons;
    if (control_at && frame_count >= control_at)
        scheduled = control_buttons;
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
           " [--frame-limit N] [--dump-vram PATH] [--pad-at FRAME:BUTTONS ...]"
           " [--warp FRAME:LEVEL] [--capture-seq DIR:FIRST:LAST] [--scale N] [--fps60] [--widescreen] [--dump-audio PATH]\n", program);
    printf("--scale renders at N times the PS1 resolution; --fps60 adds an interpolated frame between logic frames.\n");
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
            headless = 1;
            SDL_SetEnvironmentVariable(SDL_GetEnvironment(), "PSYZ_HEADLESS", "1", true);
            /* Game time follows frames, so headless runs are deterministic
             * and run as fast as the host allows. */
            SDL_SetEnvironmentVariable(SDL_GetEnvironment(), "PSYZ_VIRTUAL_CLOCK", "1", true);
            /* Automated runs must not play music on the host. */
            SDL_SetEnvironmentVariable(SDL_GetEnvironment(), "SDL_AUDIODRIVER", "dummy", true);
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
        } else if (!strcmp(argv[i], "--control-at") && i + 1 < argc) {
            control_at = (u32)strtoul(argv[++i], NULL, 10);
        } else if (!strcmp(argv[i], "--trace") && i + 1 < argc) {
            trace_period = (u32)strtoul(argv[++i], NULL, 10);
        } else if (!strcmp(argv[i], "--warp") && i + 1 < argc) {
            char *end;
            unsigned long frame, level;
            errno = 0;
            frame = strtoul(argv[++i], &end, 10);
            level = *end == ':' ? strtoul(end + 1, &end, 0) : 0x100;
            if (errno || *end || frame == 0 || frame > UINT_MAX || level > 0x3F) {
                fprintf(stderr, "Invalid --warp %s\n", argv[i]);
                return EXIT_FAILURE;
            }
            warp_frame = (u32)frame;
            warp_level = (s32)level;
        } else if (!strcmp(argv[i], "--pad-at") && i + 1 < argc) {
            if (!add_pad_event(argv[++i])) {
                fprintf(stderr, "Invalid or out-of-order --pad-at %s\n", argv[i]);
                return EXIT_FAILURE;
            }
        } else if (!strcmp(argv[i], "--fps60")) {
            cli_fps60 = 1;
        } else if (!strcmp(argv[i], "--scale") && i + 1 < argc) {
            cli_scale = (unsigned)strtoul(argv[++i], NULL, 10);
            if (cli_scale < 1 || cli_scale > 8) {
                fprintf(stderr, "--scale takes 1 to 8\n");
                return EXIT_FAILURE;
            }
        } else if (!strcmp(argv[i], "--dump-audio") && i + 1 < argc) {
            audio_path = argv[++i];
        } else if (!strcmp(argv[i], "--widescreen")) {
            g_widescreen_experimental = 1;
        } else if (!strcmp(argv[i], "--menu-at") && i + 1 < argc) {
            menu_at = (u32)strtoul(argv[++i], NULL, 10);
        } else if (!strcmp(argv[i], "--menu-capture") && i + 1 < argc) {
            char *colon = strchr(argv[++i], ':');
            if (colon == NULL) {
                fprintf(stderr, "Invalid --menu-capture, expected FRAME:PATH\n");
                return EXIT_FAILURE;
            }
            *colon = 0;
            menu_capture_frame = (u32)strtoul(argv[i], NULL, 10);
            menu_capture_path = colon + 1;
        } else if (!strcmp(argv[i], "--capture-seq") && i + 1 < argc) {
            char *spec = argv[++i];
            char *last = strrchr(spec, ':');
            char *first = last ? (*last = 0, strrchr(spec, ':')) : NULL;
            if (first == NULL) {
                fprintf(stderr, "Invalid --capture-seq, expected DIR:FIRST:LAST\n");
                return EXIT_FAILURE;
            }
            *first = 0;
            seq_dir = spec;
            seq_first = (u32)strtoul(first + 1, NULL, 10);
            seq_last = (u32)strtoul(last + 1, NULL, 10);
        } else if (!strcmp(argv[i], "--capture") && i + 1 < argc) {
            capture_path = argv[++i];
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
    if (audio_path != NULL) {
        audio_file = fopen(audio_path, "wb");
        if (audio_file == NULL || frame_limit == 0) {
            fprintf(stderr, "--dump-audio needs --frame-limit and a writable path\n");
            return EXIT_FAILURE;
        }
        fseek(audio_file, 44, SEEK_SET);
        SDL_SetEnvironmentVariable(SDL_GetEnvironment(), "PSYZ_AUDIO_NO_DEVICE", "1", true);
    }
    /* Headless runs ignore and never write the settings file. */
    g_settings_persist = !headless;
    if (g_settings_persist)
        Settings_Load();
    if (cli_fps60 >= 0)
        g_settings.fps60 = cli_fps60;
    if (cli_scale != 0)
        g_settings.scale = (int)cli_scale;
    if (headless && g_widescreen_experimental)
        g_settings.widescreen = 1; /* a headless run has no menu to ask */
    Settings_Apply();
    if (!headless)
        Psyz_EscapeIsMenu(1);
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
