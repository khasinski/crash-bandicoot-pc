#include "common.h"
#include "vector.h"
#include <math.h>

static u32 seed = 521;
static s32 next_value(s32 range)
{
    seed = seed * 1664525u + 1013904223u;
    return (s32)(seed >> 8) % range - range / 2;
}

static s32 isqrt(u32 value)
{
    u32 root = (u32)sqrt((double)value);
    while ((u64)root * root > value) --root;
    while ((u64)(root + 1) * (root + 1) <= value) ++root;
    return (s32)root;
}

static int near_root(s32 distance, u32 square)
{
    s32 root = isqrt(square);
    return (distance & 0xFF) == 0 && abs((distance >> 8) - root) <= root / 64 + 1;
}

static int test_trigonometry(void)
{
    s32 angle;
    for (angle = -0x2000; angle < 0x2000; ++angle) {
        double radians = (angle & 0xFFF) * (2.0 * 3.14159265358979323846 / 4096.0);
        if (Math_Sin(angle) != lround(4096.0 * sin(radians))) return 1;
        if (Math_Cos(angle) != lround(4096.0 * cos(radians))) return 2;
    }
    return 0;
}

static int test_angles(void)
{
    unsigned i;
    for (i = 0; i < 100000; ++i) {
        s32 current = next_value(0x4000), target = next_value(0x4000);
        s32 step = next_value(0x1000);
        s32 diff = Gool_AngDiff(current, target);
        s32 clamped = Math_AngDiffClamp(current, target, step);
        /* The shortest signed turn from current to target. */
        if (diff < -0x800 || diff > 0x800 || ((current + diff - target) & 0xFFF)) return 3;
        if (step >= 0 && step < abs(diff)) {
            if (clamped != (diff > 0 ? step : -step)) return 4;
        } else if (step >= 0 && clamped != diff) {
            return 5;
        }
    }
    return 0;
}

static int test_distances(void)
{
    unsigned i;
    /* World coordinates are 20.8 fixed point; keep squared deltas in range. */
    for (i = 0; i < 100000; ++i) {
        Vec3 a = {next_value(0x400000), next_value(0x400000), next_value(0x400000)};
        Vec3 b = {next_value(0x400000), next_value(0x400000), next_value(0x400000)};
        s32 d[3] = {abs(a.x - b.x), abs(a.y - b.y), abs(a.z - b.z)};
        s32 major = d[0] >= d[1] && d[0] >= d[2] ? 0 : d[1] >= d[2] ? 1 : 2;
        s32 sx = (a.x - b.x) >> 8, sy = (a.y - b.y) >> 8, sz = (a.z - b.z) >> 8;
        /* The approximation adds a quarter of the two minor axes. */
        if (Math_ApxDist(&a, &b) != d[major] + (d[0] + d[1] + d[2] - d[major]) / 4) return 6;
        if (Math_Absdiff(a.x, b.x) != d[0]) return 7;
        /* SquareRoot0 is the PS1 table approximation, not an exact root. */
        if (!near_root(Math_EucDist(&a, &b), (u32)(sx * sx + sy * sy + sz * sz))) return 8;
        if (!near_root(Math_EucDistXZ(&a, &b), (u32)(sx * sx + sz * sz))) return 9;
    }
    return 0;
}

static int test_random(void)
{
    u32 state = 77;
    unsigned i;
    Math_Sranda(77);
    for (i = 0; i < 100000; ++i) {
        s32 bound = next_value(0x10000) | 1;
        state = state * 1103515245u + 12345u;
        if (Math_Randa(bound) != (s32)((state / 15) % 0x07FFFFFF) % bound) return 10;
    }
    Math_Sranda2();
    Math_Srandb2();
    for (i = 0; i < 1000; ++i)
        if (Math_Randa(1000) != Math_Randb(1000)) return 11;
    if (Math_Randa(0) != 0) return 12;
    return 0;
}

static int test_seek(void)
{
    unsigned i;
    for (i = 0; i < 100000; ++i) {
        s16 current = (s16)next_value(0x10000), target = (s16)next_value(0x10000);
        s16 start = current, goal = target;
        s32 step = abs(next_value(0x2000));
        Zone_ColorSeek(&current, &target, step);
        /* Moves toward the goal by at most step, both outputs agree. */
        if (current != target) return 13;
        if (step == 0 ? current != goal
                      : (abs(goal - start) <= step ? current != goal
                                                   : abs(current - start) != step))
            return 14;
    }
    return 0;
}

int main(void)
{
    int result;
    if ((result = test_trigonometry()) || (result = test_angles()) ||
        (result = test_distances()) || (result = test_random()) ||
        (result = test_seek())) {
        printf("Original game math failed: %d\n", result);
        return result;
    }
    puts("Original game math: trigonometry, angles, distances, random and seek passed");
    return 0;
}
