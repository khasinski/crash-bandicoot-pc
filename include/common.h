#ifndef CRASH_COMMON_H
#define CRASH_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <psyz/types.h>

typedef int64_t s64;
typedef uint64_t u64;

#define CRASH_OFFSETOF(type, member) offsetof(type, member)

#include "globals.h"
#include "functions.h"

#endif
