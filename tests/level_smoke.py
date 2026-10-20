#!/usr/bin/env python3
"""Warp to each level of the disc and check that the game loads and runs it.

usage: level_smoke.py GAME [LEVEL ...]
Needs the USA disc image in CRASH_TEST_DISC (a .cue); without it the test is
skipped (exit 77). Each level runs headless: START on the title, a warp to the
level, then a trace. The level is good when the trace reports it, Crash exists
(an object address) and the process exits 0.
"""
import array
import json
import os
import subprocess
import sys
import tempfile
import wave

# Levels that exist on the disc and load (docs/PC_PORT_PLAN.md).
LEVELS = [3, 5, 6, 7, 8, 9, 10, 12, 14, 15, 17, 18, 19, 20, 21, 22, 23, 24]
FRAMES = 3000
WARP_AT = 2400
START_AT = 1800


def silent_blocks(path):
    """Half-second blocks of the last eight seconds whose peak is below 50."""
    with wave.open(path) as wav:
        samples = array.array("h")
        samples.frombytes(wav.readframes(wav.getnframes()))
    tail = samples[-2 * 44100 * 8:]
    block = 2 * 22050
    quiet = 0
    for i in range(0, len(tail) - block + 1, block):
        part = tail[i:i + block]
        quiet += max(max(part), -min(part)) < 50
    return quiet


def run(game, disc, level, sound):
    cmd = [game, "--disc", disc, "--headless", "--frame-limit", str(FRAMES),
           "--dump-audio", sound,
           "--pad-at", f"{START_AT}:0x800", "--pad-at", f"{START_AT + 10}:0",
           "--warp", f"{WARP_AT}:{level}", "--trace", "300"]
    cmd += os.environ.get("CRASH_LEVEL_ARGS", "").split()  # e.g. --widescreen
    done = subprocess.run(cmd, capture_output=True, text=True, timeout=600)
    states = []
    for line in done.stdout.splitlines():
        if line.startswith('{"frame"'):
            states.append(json.loads(line))
    return done.returncode, states


def main():
    disc = os.environ.get("CRASH_TEST_DISC")
    if not disc:
        print("CRASH_TEST_DISC is not set; skipping")
        return 77
    game = sys.argv[1]
    levels = [int(v) for v in sys.argv[2:]] or LEVELS
    failed = []
    for level in levels:
        with tempfile.TemporaryDirectory() as tmp:
            sound = os.path.join(tmp, "sound.wav")
            code, states = run(game, disc, level, sound)
            silent = silent_blocks(sound) if code == 0 else -1
        last = states[-1] if states else {}
        ok = code == 0 and last.get("level") == level and "x" in last and silent == 0
        print(f"level {level}: {'ok' if ok else 'FAILED'} (exit {code}, "
              f"{silent} silent blocks, last {last})")
        if not ok:
            failed.append(level)
    if failed:
        print("failed levels:", failed)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
