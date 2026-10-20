#!/bin/bash
# Run an oracle build (cmake -DCRASH_GTE_ORACLE=ON) over attract mode and a
# warp to every level, in parallel, and summarise the oracle reports.
#
#     tools/scripts/gte_oracle_check.sh BUILD/crash-bandicoot-pc DISC.cue OUTDIR [FRAMES]
#
# Prints the runs with a mismatch, the per-routine call counts and the runs
# that did not exit cleanly. Logs are kept in OUTDIR.
set -u
BIN=$1 DISC=$2 OUT=$3 FRAMES=${4:-1200}
LEVELS="03 05 06 07 08 09 0A 0C 0E 0F 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E
1F 20 21 22 23 24 25 26 28 29 2A 2C 2D 2E 33 34 37 38 39"
mkdir -p "$OUT"
rm -f "$OUT"/*.log
run() {
    name=$1
    shift
    "$BIN" --disc "$DISC" --headless "$@" > "$OUT/$name.log" 2>&1
    echo "exit $?" >> "$OUT/$name.log"
}
export -f run
export BIN DISC OUT
{
    echo "attract --frame-limit 9000"
    for l in $LEVELS; do
        echo "level$l --frame-limit $FRAMES --warp 120:0x$l"
    done
} | xargs -P "$(sysctl -n hw.ncpu 2>/dev/null || nproc)" -L 1 bash -c 'run "$@"' _ 2>/dev/null
echo "mismatches in: $(grep -l 'differs' "$OUT"/*.log | xargs -n1 basename 2>/dev/null | tr '\n' ' ')"
grep -h 'gte oracle: .* calls$' "$OUT"/*.log |
    awk '{n[$3] += $4} END {for (k in n) printf "  %-32s %d\n", k, n[k]}' | sort
echo "unclean exits: $(grep -L '^exit 0$' "$OUT"/*.log | xargs -n1 basename 2>/dev/null | tr '\n' ' ')"
