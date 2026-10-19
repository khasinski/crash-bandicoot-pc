#!/bin/bash
# Compile game modules with pointer/integer conversions as errors.
# usage: tools/scripts/strict_check.sh src/main/dir/file.c
cd "$(dirname "$0")/../.."
clang -std=c11 -D__psyz -c -Iinclude -Isrc/main -Ithird_party/psyz/psyz/include \
  -Wall -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-parameter \
  -Werror=int-conversion -Werror=pointer-integer-compare -Werror=pointer-to-int-cast \
  -Werror=int-to-pointer-cast -Werror=incompatible-pointer-types -Werror=implicit-function-declaration \
  -Werror=incompatible-function-pointer-types -Werror=return-type -Werror=void-pointer-to-int-cast \
  -Werror=deprecated-non-prototype -Werror=compare-distinct-pointer-types -Werror=pointer-type-mismatch \
  -ferror-limit=0 "$@" -o /dev/null
