# Crash Bandicoot PC port plan

## Current state

The repository was imported from the decompilation at `965a1bd`. Every engine
function exists as C except the hand-written scratchpad routines in
`src/main/engine/gte_render.c`; 39 of the original 41 remain to be translated.

Done so far:

- `common.h` is native: `<stdint.h>` types through PsyZ, no `__asm__`
  aliases or matching macros. PsyQ prototypes come from the PsyZ headers.
  K&R declarations in `functions.h` were replaced with full prototypes
  wherever the definition's types are visible to the header; 97 remain, for
  untranslated assembly routines and for functions whose parameter types are
  local to their `.c` file.
  Per-file historical compiler flags were removed.
- Call sites that passed ignored extra arguments, or relied on a register
  passing through, now pass the real arguments (`Gfx_UpdateMatrices2` takes
  the ordering table its original forwarded in a0).
- `Math_Sin`/`Math_Cos` (originally `sin`/`cos` in hand-written assembly) are
  C. Their quarter-wave table is `round(4096 * sin(i * pi / 2048))`, which
  equals all 1025 retail entries at 0x8005306C, so no retail data is copied.
- The LCG generators wrap in unsigned arithmetic instead of overflowing.
- `crash-original-math` builds with `-Wall -Wextra` and no warnings, and
  `original-game-math` passes, also under ASan and UBSan.

Natively compiling (not yet joined, `clang -w`): 120 of 155 engine units; 50
with no warnings. The rest mostly fail on per-file struct views of the same
object (`GoolObj` is declared in nine files with different partial layouts),
PS1 GPU primitive declarations in `include/gpu.h` that conflict with PsyZ, and
`asm` register pins.

### Data

The decompilation compiled all code from C, but 1225 data symbols (452 in
BSS) are still defined only in the generated assembly of the retail
executable. The native build needs a C definition for every global it
uses. BSS needs only a typed definition. Initialized data needs one of:

- a C initializer, when the values are code-derived (pointers, dispatch
  tables, small constants) or computable, as with the sine table;
- decoding from `SCUS_949.00` on the user's disc at startup, for large
  opaque tables, so that no retail bytes enter the repository.

Each module defines the data it owns when it joins the build.

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
