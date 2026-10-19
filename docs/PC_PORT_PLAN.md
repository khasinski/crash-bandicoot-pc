# Crash Bandicoot PC port plan

## Current state

The repository was imported from the decompilation at `965a1bd`. Every engine
function exists as C except the 41 hand-written scratchpad routines in
`src/main/engine/gte_render.c`. The native build compiles PsyZ, SDL and the
original modules listed in `CMakeLists.txt`. The original entry does not run
yet.

## Milestones

Modules join the native target in dependency order. Each step must build
without warnings in the module, pass its native tests, and leave every
earlier test passing.

1. **Foundation.** Host-portable `common.h` (fixed-width types, no `__asm__`
   aliases or matching macros), PsyZ and SDL submodules, the CMake target, and
   pure helpers (`math/`, `libc/malloc.c`) with reference tests.
2. **Disc access.** Read ISO 9660 files from a user-supplied CUE/BIN image
   (MODE2/2352, one data track) and serve the original CD file-lookup and
   sector-read entry points (`engine/cd_dir.c`, `engine/cd_read.c`).
   Validate the image by the `SCUS_949.00` identity in `SYSTEM.CNF`.
3. **NS paging.** Port `ns/` to host memory: a host page pool, NSD index
   decoding into host structures, and entry pointers that never pass through
   32-bit words. Test it by loading every level's NSD and paging in all entries
   from the real image (skipped without `CRASH_TEST_DISC`).
4. **Scratchpad routines.** Translate the 41 routines in `gte_render.c` to C
   on the PsyZ GTE API. Test each one against reference results recorded from
   the original routine for real level data.
5. **GOOL.** Port the object pool and interpreter. Decide the representation of
   object references in object fields (pool handles instead of raw pointers)
   and apply it consistently in `gool/`, `level/` and `cam/`. Test the
   interpreter by running GOOL executables from the disc headlessly.
6. **Rendering.** Port `render/`, ordering tables (`OT_TYPE`) and primitive
   structures (`O_TAG`) for PsyZ, and the frame/display loop.
7. **Boot.** Start `CORE_Main` from `main()` and reach the title screen
   headlessly, with a `--headless --frame-limit N --dump-vram PATH` diagnostic
   boundary.
8. **Audio and input.** SPU voices, MIDI sequences, CD audio through PsyZ;
   SDL keyboard and gamepad mapped to the original pad packet.
9. **Gameplay.** Reach and complete levels, with saves (memory card or
   password), and verify against the original game.

High resolution comes through PsyZ internal-resolution scaling. High frame
rate is a later milestone; presentation must never change simulation.

## Acceptance

A passing source-only build does not establish gameplay. A feature is complete
when it has behavior assertions and a result on real content. Record the
evidence (test name, frame count, VRAM capture) in this file when a milestone
closes.
