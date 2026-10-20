#!/usr/bin/env python3
"""Warp to each level of the disc and check that the game loads and runs it.

usage: level_smoke.py GAME [LEVEL ...]
Needs the USA disc image in CRASH_TEST_DISC (a .cue); without it the test is
skipped (exit 77). Each level runs headless: START on the title, a warp to the
level, then a trace. The level is good when the trace reports it, Crash exists
(an object address) and the process exits 0.
"""
import json
import os
import subprocess
import sys

# Levels that exist on the disc and load (docs/PC_PORT_PLAN.md).
LEVELS = [3, 5, 6, 7, 8, 9, 10, 12, 14, 15, 17, 18, 19, 20, 21, 22, 23, 24]
FRAMES = 3000
WARP_AT = 2400
START_AT = 1800


def run(game, disc, level):
    cmd = [game, "--disc", disc, "--headless", "--frame-limit", str(FRAMES),
           "--pad-at", f"{START_AT}:0x800", "--pad-at", f"{START_AT + 10}:0",
           "--warp", f"{WARP_AT}:{level}", "--trace", "300"]
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
        code, states = run(game, disc, level)
        last = states[-1] if states else {}
        ok = code == 0 and last.get("level") == level and "x" in last
        print(f"level {level}: {'ok' if ok else 'FAILED'} (exit {code}, last {last})")
        if not ok:
            failed.append(level)
    if failed:
        print("failed levels:", failed)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
