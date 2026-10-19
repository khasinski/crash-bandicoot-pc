#include "common.h"

/* =========================================================================
 * Section 1: CD Directory Reader (asm/USA/main/text_200C4.s)
 * ========================================================================= */

typedef struct NsFileMapBits {
    u32 directory_sector : 23;
    u32 reserved_0 : 9;
    u32 directory_size : 17;
    u32 reserved_1 : 15;
    u32 page_sector : 24;
    u32 reserved_2 : 8;
} NsFileMapBits;

#define CRASH_EXT_ENGINE_CD_DIR
#include "engine_externs.h"
#undef CRASH_EXT_ENGINE_CD_DIR

static __inline__ s32 cd_read_sectors(s32 sector_count, s32 first_sector, void *destination, CdlLOC *position) {
    CdIntToPos(first_sector, position);
    CdControl(2, (u_char *)position, NULL);
    CdRead(sector_count, (u_long *)destination, 0x80);
    return CdReadSync(0, NULL) == 0;
}

static __inline__ void *cd_memcpy(void *dst, const void *src, s32 len) {
    dst = BIOS_MemCpy(dst, src, len);
    return dst;
}

s32 Cd_ReadFileSys(void) {
    u8 name[32];
    s32 subdirs[4];
    CdlLOC position;
    s32 sector;
    s32 pt_sector;
    s32 size;
    u8 *buf;
    u8 *p;
    s32 i;
    s32 j;
    s32 level;

    buf = LIBC_Malloc(0x800);
    if (cd_read_sectors(1, 0x10, buf, &position) != 1) {
        goto fail;
    }
    if (BIOS_StrNCmp((char *)(buf + 1), gp_cd_name, 5) != 0) {
        goto fail;
    }
    BIOS_Printf(g_str_cd_dir_reading_file_system_n);
    __builtin_memcpy(&sector, buf + 0x8C, 4);
    if (cd_read_sectors(1, sector, buf, &position) != 1) {
        goto fail;
    }
    p = buf;
    for (j = 0; j < 4; j++) {
        subdirs[j] = -1;
    }
    i = 0;
    while (p < buf + 0x800) {
        s32 len;
        s32 step;
        if (*p == 0) {
            break;
        }
        cd_memcpy(name, p + 8, *p);
        name[*p] = 0;
        __builtin_memcpy(&pt_sector, p + 2, 4);
        if (BIOS_StrLen((char *)name) == 2 && (name[0] == 'S' || name[0] == 's')) {
            subdirs[name[1] - '0'] = pt_sector;
        }
        len = *p;
        i++;
        step = (len & 1) + 8;
        p += len + step;
        if (i >= 0x80) {
            break;
        }
    }
    for (j = 0; j < 4; j++) {
        if (subdirs[j] != -1) {
            if (cd_read_sectors(1, subdirs[j], buf, &position) != 1) {
                goto fail;
            }
        }
        p = buf;
        i = 0;
        while (p < buf + 0x800) {
            if (*p == 0) {
                break;
            }
            __builtin_memcpy(&sector, p + 2, 4);
            __builtin_memcpy(&size, p + 10, 4);
            switch (i) {
            case 0:
                __builtin_memcpy(name, &gp_iso_dot_rec_name, 2);
                break;
            case 1:
                __builtin_memcpy(name, &gp_iso_ddot_rec_name, 3);
                break;
            default:
                cd_memcpy(name, p + 0x21, p[0x20]);
                name[p[0x20]] = 0;
                break;
            }
            if (name[0] == 'S' || name[0] == 's') {
                if (name[7] >= 'A') {
                    level = (name[6] - '0') * 16 + (name[7] - 'A' + 10);
                } else {
                    level = (name[6] - '0') * 16 + (name[7] - '0');
                }
                if (name[11] == 'D' || name[11] == 'd') {
                    g_file_map[level].directory_sector = sector;
                    g_file_map[level].directory_size = size;
                } else {
                    g_file_map[level].page_sector = sector;
                }
            }
            i++;
            p += *p;
            if (i >= 0x40) {
                break;
            }
        }
    }
    LIBC_Free(buf);
    return 1;
fail:
    LIBC_Free(buf);
    return 0;
}

