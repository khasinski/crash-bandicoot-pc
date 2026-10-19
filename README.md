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

The port has just started. The native build compiles PsyZ and the original
math modules, with `Math_Sin`/`Math_Cos` translated from the hand-written
assembly, and `tests/original_math.c` checks them. Of the 155 engine
translation units, 120 already compile natively, but most still assume 32-bit
pointers and the PS1 data layout. The original game entry does not run yet,
so there is no `crash-bandicoot-pc` executable yet. See
[the port plan](docs/PC_PORT_PLAN.md) for the order in which subsystems join
the build.

## Build and run

Use a C11 compiler, CMake 3.21 or newer and Git. PsyZ and SDL are source
submodules. You do not need a historical PS1 compiler, the Sony SDK or an
extracted game executable.

```sh
git clone https://github.com/khasinski/crash-bandicoot-pc
cd crash-bandicoot-pc
git submodule update --init third_party/psyz
git -C third_party/psyz submodule update --init external/SDL
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure --no-tests=error
```

Once the boot milestone lands, the game will start with
`./build/crash-bandicoot-pc --disc "/path/to/Crash Bandicoot (USA).cue"` and
read its data from that user-supplied CUE/BIN image. The image must be the USA
release (redump 2250, track 1 SHA-1
`61ff4c9e8dcfb9e5d94d37f66330b69d08d6c914`).

Tests that need game data will read the image named by `CRASH_TEST_DISC`
and skip with code 77 when it is unset.

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
