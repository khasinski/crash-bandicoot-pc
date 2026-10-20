# Changelog

## 0.1.0

First release of the native PC port (macOS and Linux).

- Native C11 game on the PsyZ runtime; the original MIPS code is kept only as
  a test oracle.
- Runs from the user's own disc image (`--disc <cue>`); levels 3, 5–10, 12, 14,
  15 and 17–24 and the title/map load. Level 4 is an unused stub on the disc.
- Escape opens a menu: 60 FPS, resolution (1–8×), resume, quit. Settings are
  stored in `crash-bandicoot-pc.ini`.
- `--fps60` interpolates between logic frames; `--scale N` renders at N× the
  PS1 resolution. Neither changes the simulation (checked by tests).
- `--fps60` no longer interpolates the title and map screen (level 25), where it
  made static parts jitter by a pixel every other frame.
- Headless mode (`--headless`, `--capture-seq`, `--dump-audio`) for tests.
- Real-disc tests (`CRASH_TEST_DISC`): every level loads and has sound;
  presentation options leave the simulation identical.

- Widescreen: ASPECT RATIO in the ESC menu (4:3 or 16:9, `--widescreen` starts in
  16:9). The world adds the polygons the baked 4:3 lists lack, and the primitive
  pools moved to spare RAM to hold them.

Known limits: widescreen can show a small wedge of background colour in a top
corner, and the fog and lightning world passes stay 4:3; music fidelity against the console is unverified; no full
playthrough or save verification; Windows is not supported.
