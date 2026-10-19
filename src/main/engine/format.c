#include "common.h"

typedef union FormatFlags {
    u32 bits;
    struct {u8 flags; u8 sign; u8 reserved[2];} bytes;
} FormatFlags;
typedef struct FormatSpec {FormatFlags flags; s32 width; s32 precision;} FormatSpec;
#define CRASH_EXT_ENGINE_FORMAT
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_FORMAT
#include <stdarg.h>

/* The game's own formatter (the PsyQ sprintf). It has its own name so it
 * does not replace the host's; game code reaches it through the sprintf
 * macro in functions.h. Arguments are read with va_arg instead of walking
 * the MIPS stack words. */
int LIBC_Sprintf(char *arg0, const char *format_string, ...) {
    const char *arg1 = format_string;
    s32 plus,minus,space;
    s32 alternate; s32 zero;
    u8 digits[512];
    FormatSpec format;
    va_list arguments;
    s32 var_s2;
    u8 *var_s1;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 var_s0;
    s32 var_v0_3;
    const char *temp_a0;
    const char *temp_a0_2;
    const char *temp_v1_10;
    const char *temp_v1_2;
    const char *temp_v1_4;
    const char *temp_v1_5;
    const char *var_a3;
    s8 *var_v1_2;
    s8 *var_v1_3;
    s8 temp_v0_2;
    u32 temp_v1_6;
    u32 temp_v1_8;
    u32 temp_v1_9;
    u32 temp_v0;
    u32 var_a0;
    u32 var_v0_2;
    u32 var_v1;
    u32 argument_flags;
    s32 var_a1;
    void *temp_v0_8;

    va_start(arguments, format_string);
    var_a1 = (u8) arg1[0];
    var_s2 = 0;
    if (var_a1 != 0) {minus=0x2D;plus=0x2B;space=0x20;
loop_2:
        if (var_a1 != 0x25) goto block_108; {
            format = g_sprintf_thousands_table;alternate=0x23;zero=0x30;
loop_4:
            temp_a0 = arg1;
            arg1 = temp_a0 + 1;
            var_a1 = (u8)temp_a0[1];
            if (var_a1 == minus) {
                format.flags.bits = (s32) format.flags.bits | 1;
                goto loop_4;
            }
            if (var_a1 == plus) {
                format.flags.bits |= 2;
                goto loop_4;
            }
            if (var_a1 == space) {
                format.flags.bytes.sign = var_a1;
                goto loop_4;
            }
            if (var_a1 == alternate) {
                format.flags.bits |= 4;
                goto loop_4;
            }
            if (var_a1 == zero) {
                format.flags.bits |= 8;
                goto loop_4;
            }
            if (var_a1 == 0x2A) {
                temp_v0 = va_arg(arguments, u32);
                format.width = temp_v0;
                if ((s32) temp_v0 < 0) {
                    format.width = -(s32) temp_v0;
                    format.flags.bits |= 1;
                }
                arg1 = temp_a0 + 2;
                var_a1 = (u8)temp_a0[2];
            } else {
                while ((u32)(var_a1-0x30) < 10) {
                    {s32 decimal=format.width*10;decimal-=0x30;decimal+=var_a1;format.width=decimal;}
                    temp_v1_2 = arg1;
                    arg1 = temp_v1_2 + 1;
                    var_a1 = (u8)temp_v1_2[1];
                }
            }
            if (var_a1 == 0x2E) {
                temp_a0_2 = arg1;
                arg1 = temp_a0_2 + 1;
                var_a1 = (u8)temp_a0_2[1];
                if (var_a1 == 0x2A) {
                    format.precision = va_arg(arguments, s32);
                    arg1 = temp_a0_2 + 2;
                    var_a1 = (u8)temp_a0_2[2];
                } else {
                    var_v0_2=var_a1-0x30;
                    while (var_v0_2 < 10) {
                        {s32 decimal=format.precision*10;decimal-=0x30;decimal+=var_a1;format.precision=decimal;}
                        temp_v1_4 = arg1;
                        arg1 = temp_v1_4 + 1;
                        var_a1 = (u8)temp_v1_4[1];
                        var_v0_2=var_a1-0x30;
                    }
                }
                if (format.precision >= 0) {
                    format.flags.bits |= 0x10;
                }
            }
            var_s1 = digits + 512;
            if (format.flags.bits & 1) {
                format.flags.bits &= ~8;
            }
            var_v1 = var_a1 - 0x4C;
loop_30:
            switch (var_v1) {
            case 28:
                var_v0_3 = format.flags.bits | 0x20;
                goto block_35;
            case 32:
                var_v0_3 = format.flags.bits | 0x40;
                goto block_35;
            case 0:
                var_v0_3 = format.flags.bits | 0x80;
block_35:
                format.flags.bits = var_v0_3;
                temp_v1_5 = arg1;
                arg1 = temp_v1_5 + 1;
                var_a1 = (u8)temp_v1_5[1];
                var_v1 = var_a1 - 0x4C;
                goto loop_30;
            case 24:
            case 29:
                var_a0 = va_arg(arguments, u32);
                temp_v1_6 = format.flags.bits;
                if (temp_v1_6 & 0x20) {
                    var_a0 = (u32) (s16) var_a0;
                }
                if ((s32) var_a0 < 0) {
                    var_a0 = (u32) -(s32) var_a0;
                    format.flags.bytes.sign = minus;
                } else if (temp_v1_6 & 2) {
                    format.flags.bytes.sign = plus;
                }
                goto block_44;
            case 41:
                {
                    u32 flags_41 = format.flags.bits;
                    var_a0 = va_arg(arguments, u32);
                    format.flags.bytes.sign = 0;
                    if (flags_41 & 0x20) {
                        var_a0 &= 0xFFFF;
                    }
                }
block_44:
                if (!(format.flags.bits & 0x10)) {
                    if (format.flags.bits & 8) {
                        format.precision = format.width;
                        if (format.flags.bytes.sign != 0) {
                            format.precision = format.width - 1;
                        }
                    }
                    if (format.precision <= 0) {
                        format.precision = 1;
                    }
                }
                var_s0 = 0;
                if (var_a0 != 0) {
                    do {
                        var_s1 -= 1;
                        var_s0 += 1;
                        temp_v0_2 = (var_a0 % 10) + 0x30;
                        var_a0 /= 0xA;
                        *var_s1 = temp_v0_2;
                    } while (var_a0 != 0);
                }
                if (var_s0 < format.precision) {
                    s32 pad_zero=0x30;
                    var_s1 = var_s1 - 1;
                    do {
                        *var_s1 = pad_zero;
                        var_s1 -= 1;
                    } while (++var_s0 < format.precision);
                    var_s1 = var_s1 + 1;
                }
                if (format.flags.bytes.sign != 0) {
                    var_s1 -= 1;
                    var_s0 += 1;
                    *var_s1 = format.flags.bytes.sign;
                }
                goto block_109;
            case 35:
                var_a0 = va_arg(arguments, u32);
                temp_v1_8 = format.flags.bits;
                if (temp_v1_8 & 0x20) {
                    var_a0 &= 0xFFFF;
                }
                if (!(temp_v1_8 & 0x10)) {
                    if (temp_v1_8 & 8) {
                        format.precision = format.width;
                    }
                    if (format.precision <= 0) {
                        format.precision = 1;
                    }
                }
                var_s0 = 0;
                if (var_a0 != 0) {
                    do {
                        var_s1 -= 1;
                        *var_s1 = (s8) ((var_a0 & 7) + 0x30);
                        var_a0 = var_a0 >> 3;
                        var_s0 += 1;
                    } while (var_a0 != 0);
                }
                if ((format.flags.bits & 4) && (var_s0 != 0) && (*var_s1 != 0x30)) {
                    var_s1 -= 1;
                    *var_s1 = 0x30;
                    var_s0 += 1;
                }
                if (var_s0 < format.precision) {
                    s32 pad_zero=0x30;
                    var_s1 = var_s1 - 1;
                    do {
                        *var_s1 = pad_zero;
                        var_s1 -= 1;
                    } while (++var_s0 < format.precision);
                    var_s1 = var_s1 + 1;
                }
                goto block_109;
            case 36:
                format.precision = 8;
                format.flags.bits |= 0x50;
                /* fallthrough */
            case 12:
                var_a3 = g_str_hex_digits_lower;
                goto block_79;
            case 44:
                var_a3 = g_str_hex_digits_upper;
block_79:
                var_a0 = va_arg(arguments, u32);
                temp_v1_9 = format.flags.bits;
                if (temp_v1_9 & 0x20) {
                    var_a0 &= 0xFFFF;
                }
                if (!(temp_v1_9 & 0x10)) {
                    if (temp_v1_9 & 8) {
                        format.precision = format.width;
                        if (temp_v1_9 & 4) {
                            format.precision = format.width - 2;
                        }
                    }
                    if (format.precision <= 0) {
                        format.precision = 1;
                    }
                }
                var_s0 = 0;
                if (var_a0 != 0) {
                    do {
                        var_s1 -= 1;
                        temp_v0_4 = var_a0 & 0xF;
                        var_a0 = var_a0 >> 4;
                        var_s0 += 1;
                        *var_s1 = (u8) var_a3[temp_v0_4];
                    } while (var_a0 != 0);
                }
                if (var_s0 < format.precision) {
                    s32 pad_zero=0x30;
                    var_s1 = var_s1 - 1;
                    do {
                        *var_s1 = pad_zero;
                        var_s1 -= 1;
                    } while (++var_s0 < format.precision);
                    var_s1 = var_s1 + 1;
                }
                if (format.flags.bits & 4) {
                    var_s1--;
                    *var_s1 = var_a1;
                    var_s1--;
                    var_s0 += 2;
                    *var_s1 = 0x30;
                }
                goto block_109;
            case 23:
                var_s1 -= 1;
                var_s0 = 1;
                *var_s1 = (u8) va_arg(arguments, u32);
                goto block_109;
            case 39:
                var_s1 = va_arg(arguments, u8 *);
                if (format.flags.bits & 4) {
                    var_s0 = (u8)*var_s1;
                    var_s1 += 1;
                    if (((format.flags.bits & 0x10) != 0) && (format.precision < var_s0)) {
                        var_s0 = format.precision;
                    }
                } else if ((format.flags.bits & 0x10) == 0) {
                    var_s0 = BIOS_StrLen((const char *) var_s1);
                } else {
                    temp_v0_8 = BIOS_MemChr(var_s1, 0, format.precision);
                    var_s0 = (u8 *)temp_v0_8 - var_s1;
                    if (temp_v0_8 == NULL) {
                        var_s0 = format.precision;
                    }
                }
                goto block_109;
            case 34:
                var_s1 = va_arg(arguments, u8 *);
                if (format.flags.bits & 0x20) {
                    temp_v0_3 = format.flags.bits << 16;
                    *(s16 *)var_s1 = var_s2 | temp_v0_3;
                } else {
                    *(s32 *)var_s1 = (s32)var_s2;
                }
                goto block_118;
            default: ;
            }
        }
        do {
            if (var_a1 == 0x25) {
                goto block_108;
            }
            goto done;
block_108:
            arg0[var_s2] = var_a1;
            var_s2 += 1;
            goto block_118;
block_109:
                if (var_s0 < format.width) {
                    if (!(format.flags.bits & 1)) {
                        var_v1_2 = (s8 *)arg0 + var_s2;
                        do {
                            *var_v1_2 = space;
                            var_v1_2 += 1;
                            temp_v0_3 = format.width - 1;
                            format.width = temp_v0_3;
                            var_s2 += 1;
                        } while (var_s0 < temp_v0_3);
                            }
                }
                LIBC_Memmove((u8 *)&arg0[var_s2], var_s1, var_s0);
                var_s2 += var_s0;
                if (var_s0 < format.width) {
                    var_v1_3 = (s8 *)arg0 + var_s2;
                    do {
                        *var_v1_3 = space;
                        var_v1_3 += 1;
                        var_s2 += 1;
                    } while (++var_s0 < format.width);
                }
                goto block_118;
block_118:
            temp_v1_10 = arg1;
            arg1 = temp_v1_10 + 1;
            var_a1 = (u8)temp_v1_10[1];
            if (var_a1 != 0) {
                goto loop_2;
            }
        } while (var_a1 != 0);
    }
done:
    va_end(arguments);
    arg0[var_s2] = 0;
    return var_s2;
}
