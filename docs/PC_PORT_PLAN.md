# Crash Bandicoot PC port plan

## Current state

The game boots from the user's disc image to the title and the island map,
loads levels and plays them, natively. Every engine function is C, including
the former hand-written scratchpad routines, which are checked against the
original by the GTE oracle (docs/ARCHITECTURE.md). Game data lives in a PS1
RAM image (docs/MEMORY_MODEL.md) and rendering goes through PsyZ.

Levels were checked by warping to each ID from the map and running 1200 frames
(`--warp`, `--trace`), headless, on 2026-10-20:

- Load and run with Crash placed: 3, 5-10, 12, 14, 15, 17-24, and 25 (the
  title and map). Crash moves on 10 and 17 in the unattended run; the others
  were not played.
- No file on the disc, so IDs that the game never requests: 0, 1, 2, 11, 13,
  16.
- Level 4 (`S0000004.NSD`) is an unused stub: its directory is 0x870 bytes
  where real levels have 0x10000 or more, and its entry table is empty.
  Retail never loads it, and warping to it stops at the first missing entry
  (a null lookup in `Ns_EIDType`; the PS1 would read low RAM instead).

Not done: complete music (PsyZ's libsnd sequencer), play-through of every level
with saves, widescreen (see below), and behavior tests on real data in
`tests/` (only `original_math.c` and the GOOL Lisp test exist).

### Options at run time

Escape opens a pause menu (src/platform/menu.c) with 60 FPS and resolution,
stored in `crash-bandicoot-pc.ini`. `--capture-seq DIR:FIRST:LAST` writes each
presented frame as a PPM; it found the `--fps60` flicker fixed in this
revision: a fade overlay missing from interpolated frames, a mirrored model
(negative X scale) collapsing to a line halfway through a turn, and
interpolation running while the loading image was still on screen. Rerun it
after any change to `src/platform/interp.c`, and compare 30 fps output
frame by frame (it must not change).

### Widescreen

Not shippable yet. PsyZ can squeeze the projected X (`Psyz_GteSetXScale`) and
force a 16:9 output (`Psyz_VideoSetAspectRatio`), which gives the wider view
(`--widescreen`, experimental). The world, however, is drawn from polygon lists
baked per camera-path point for the 4:3 view (`Slst_Update`,
`src/main/level/update.c`), so the sides of a wider view are missing polygons.
It needs a visibility pass of its own that culls the zone's polygons against
the wide frustum and sorts them by depth (the draw loop assumes a
back-to-front list, see `World_Link`).

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
4. **Scratchpad routines.** Done: rewritten in C on the PsyZ GTE API and
   checked call by call against a translation of the original
   (`-DCRASH_GTE_ORACLE=ON`, `tools/scripts/gte_oracle_check.sh`).
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

Status: 1-8 work (audio is incomplete: music), 9 is partial (see Current
state). High resolution comes through PsyZ internal-resolution scaling and
high frame rate through interpolation between logic frames (`--fps60`);
presentation never changes simulation, and 30 fps output is the same frame
for frame with the option off.

## Acceptance

A passing source-only build does not establish gameplay. A feature is complete
when it has behavior assertions and a result on real content. Record the
evidence (test name, frame count, VRAM capture) in this file when a milestone
closes.
