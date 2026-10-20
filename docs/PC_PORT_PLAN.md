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

Behavior tests on real content (`game-levels`, `game-presentation`, README)
pass: every level that exists loads and runs, and `--fps60` and `--scale` do
not change the simulation.

Music: `--dump-audio PATH` writes the SPU output once per presented frame. On
every level that loads there is continuous sound (no silent half second in the
last eight seconds, checked by `game-levels`), and levels 15 and 24 produce the
same signal. Whether the notes and timing match the console is not verified.

Not done: a comparison of the music with real hardware, play-through of every
level with saves, and widescreen (see below).

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

The ESC menu has ASPECT RATIO (4:3 or 16:9; `--widescreen` starts in 16:9). In
16:9 PsyZ squeezes the projected X by 3/4 (`Psyz_GteSetXScale`) and stretches
the output to 16:9 (`Psyz_VideoSetAspectRatio`), in 3D gameplay only; the
title, map and loads stay 4:3.

The world is drawn from polygon lists baked per camera-path point for the 4:3
view (`Slst_Update`, `src/main/level/update.c`), so a wider view lacks the
polygons at the sides. `World_VisibleList` (`src/main/render/world_transform.c`)
keeps the baked list in order (a polygon is never linked behind the previous
one, so the order shapes the picture) and adds the zone's other polygons that
face the camera, are on screen (the screen is 512 wide: x within 256) and no
farther than the baked reach, merged by depth. The beach gains about 3800
candidates over the 1400 baked ones.

The primitive pools (`Gpu_SetupPrims`) held 0x16000 bytes of heap each, about
1550 polygons, and the 2 MB heap has no room for more. They now live in the
host's spare RAM above the PS1's 2 MiB (0x80300000, 0x60000 bytes each;
ordering-table links reach 8 MiB). The heap block is still taken so the rest of
the game's memory stays where it was; 4:3 frames are bit-identical to before.

Checked: every loadable level runs 3000 frames in 16:9 (`CRASH_LEVEL_ARGS=
--widescreen` for `tests/level_smoke.py`), and the beach and several other
levels show the whole screen. Known flaws: a small wedge of background colour
in a top corner of some views (the sky and backdrop are drawn for the 4:3
range), and some views (a few indoor and cave levels) show black strips at a
side where the zone has no more geometry or the extra polygons do not reach.
The fog, lightning and dark world passes (`world_transform_fx.c`) take the same
widened list.

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
