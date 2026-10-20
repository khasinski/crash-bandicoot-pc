#!/usr/bin/env python3
"""--fps60 and --scale must not change the simulation.

usage: presentation_smoke.py GAME
Runs N. Sanity Beach (START on the title, ✕ on the map, then walking) once
plain and once per option, headless, and requires the traces of the player's
position and state to be identical. Needs CRASH_TEST_DISC; skipped (77)
without it.
"""
import os
import subprocess
import sys

BASE = ["--frame-limit", "4200", "--trace", "100",
        "--pad-at", "1800:0x800", "--pad-at", "1810:0",
        "--pad-at", "3000:0x40", "--pad-at", "3010:0",
        "--pad-at", "3300:0x1000", "--pad-at", "3700:0x1040",
        "--pad-at", "3760:0x1000", "--pad-at", "4100:0"]
OPTIONS = {"plain": [], "fps60": ["--fps60"], "scale2": ["--scale", "2"],
           "fps60+scale2": ["--fps60", "--scale", "2"]}


def trace(game, disc, extra):
    cmd = [game, "--disc", disc, "--headless"] + BASE + extra
    done = subprocess.run(cmd, capture_output=True, text=True, timeout=900)
    if done.returncode != 0:
        raise SystemExit(f"{' '.join(extra) or 'plain'}: exit {done.returncode}")
    return [l for l in done.stdout.splitlines() if l.startswith('{"frame"')]


def main():
    disc = os.environ.get("CRASH_TEST_DISC")
    if not disc:
        print("CRASH_TEST_DISC is not set; skipping")
        return 77
    game = sys.argv[1]
    reference = trace(game, disc, OPTIONS["plain"])
    if len(reference) < 30:
        print("the reference run produced too few trace lines")
        return 1
    bad = 0
    for name, extra in OPTIONS.items():
        if name == "plain":
            continue
        got = trace(game, disc, extra)
        same = got == reference
        print(f"{name}: {'identical' if same else 'DIFFERENT'} ({len(got)} samples)")
        bad += not same
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
