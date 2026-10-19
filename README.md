# Crash Bandicoot PC

A native C11 PC port of the North American PlayStation release of Crash
Bandicoot (`SCUS-94900`), using
[our PsyZ source fork](https://github.com/khasinski/parasite-eve-psyz) for the
PlayStation runtime APIs.

The code was imported from
[crash-bandicoot-decomp](https://github.com/khasinski/crash-bandicoot-decomp)
through revision `965a1bd`. That decompilation rebuilds the retail executable
byte for byte from C, apart from 41 hand-written scratchpad routines. From that
revision onward this repository develops independently as a native C project.
Byte matching, retail checksums, historical compiler output, fixed code
addresses and PS1 binary builds are no longer acceptance requirements. Changes
are judged by behavior, portability, readability and native builds.

## Status

The original game runs natively. From the user's disc image it boots
through the Sony and Naughty Dog screens to the title menu and the island
map, loads levels and plays them: N. Sanity Beach runs with Crash moving,
the camera following and collisions working, rendered by PsyZ (SDL GPU, or
its software rasterizer headless). Music is not complete yet: PsyZ's libsnd
sequencer is still being filled in. See [the port plan](docs/PC_PORT_PLAN.md)
for what is verified and what remains.

How it works:

- Game data lives in a PS1 RAM image with every global at its original
  address; pointers stored in game memory stay PS1 addresses
  ([docs/MEMORY_MODEL.md](docs/MEMORY_MODEL.md)).
- The engine's hand-written R3000 routines are translated to C by
  `tools/scripts/gen_gte_render.py`.
- GPU packets stay in PS1 format and reach PsyZ as GP0 commands.

## Build and run

Use a C11 compiler, CMake 3.21 or newer and Git. PsyZ and SDL are source
submodules. You do not need a historical PS1 compiler, the Sony SDK or an
extracted game executable.

```sh
git clone https://github.com/khasinski/crash-bandicoot-pc
cd crash-bandicoot-pc
git submodule update --init third_party/psyz
git -C third_party/psyz submodule update --init external/SDL
cmake -S . -B build
cmake --build build --parallel
./build/crash-bandicoot-pc --disc "/path/to/Crash Bandicoot (USA).cue"
```

The game reads its data from the user-supplied CUE/BIN image. The image must
be the USA release (redump 2250, track 1 SHA-1
`61ff4c9e8dcfb9e5d94d37f66330b69d08d6c914`).

`--scale N` renders 3D at N times the PS1 resolution (1 to 8). `--fps60`
shows 60 frames per second: the game logic still runs at the original 30 Hz
and an extra frame, with the camera and objects halfway between two logic
frames, is drawn in between (src/platform/interp.c). Simulation, timing and
animations are exactly those of 30 frames per second.

Controls (PsyZ defaults): arrow keys move, X is ✕ (jump), Z is □ (spin),
D is ○, S is △, Enter is START, Backspace is SELECT, Q/R are L1/R1 and
W/E are L2/R2. SDL game controllers work as a PlayStation pad. F4 toggles
fullscreen; Escape or closing the window quits.
Memory card saves are written to the `bu00/` and `bu10/` directories in the
working directory.

Diagnostics for automated runs: `--headless` renders without a window,
`--frame-limit N` stops after N frames and `--dump-vram PATH` then writes the
1024×512 VRAM (convert with `tools/scripts/vram_to_png.py`). `--pad-at
FRAME:BUTTONS` holds a PS1 button mask from a frame on (START 0x800, ✕ 0x40,
up 0x1000), and `--warp FRAME:LEVEL` requests a level change, e.g.:

```sh
./build/crash-bandicoot-pc --disc "Crash Bandicoot (USA).cue" --headless \
    --frame-limit 3600 --pad-at 1800:0x800 --pad-at 1810:0 \
    --pad-at 3000:0x40 --pad-at 3010:0 --dump-vram beach.bin
```

`cmake -DBUILD_TESTING=ON` builds the tests in `tests/`.

`tools/scripts/gool_lisp.py` decompiles GOOL bytecode in NSF files to a Lisp
form and assembles it back (see [docs/GOOL_LISP.md](docs/GOOL_LISP.md)).

## Layout

- `src/main/` holds the original game modules, which are ported in place.
  `sdk/` holds the reconstructed PsyQ libraries; PsyZ replaces them in the
  native build, and they stay here as a behavioral reference.
- `include/` holds game types, data formats and legacy platform interfaces.
- `third_party/psyz/` is our PsyZ source fork with its SDL dependency.
- `docs/` covers the native architecture, conventions, port plan and formats.

See [CONTRIBUTING.md](CONTRIBUTING.md) and
[the port plan](docs/PC_PORT_PLAN.md) for the development workflow.

This repository contains source only. It does not include disc images, game
assets, memory cards or proprietary SDK files. Running the game requires your
own legal copy.

## Prior work used as reference

- [wurlyfox/c1](https://github.com/wurlyfox/c1): a non-matching C
  reimplementation of the engine, with names and a memory map.
- [cbhacks/CrashEdit](https://github.com/cbhacks/CrashEdit): NSD/NSF and entry
  formats.

c1 has no license, so this project takes only facts from it (names, formats,
behavior), never code.
