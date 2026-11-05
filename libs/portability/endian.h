#pragma once
// endian.h shim for toolchains whose C library omits it (e.g. MinGW-w64 GCC,
// which unlike MSVC does not take hash-library's _MSC_VER branch).
// Only the __BYTE_ORDER family is provided: every Windows target bobfilez
// supports (x86, x64, ARM64) is little-endian.
// Resolves via fo_core's existing PRIVATE ${LIBS_DIR} include path.
#include <stdint.h>
#ifndef __BYTE_ORDER
#define __LITTLE_ENDIAN 1234
#define __BIG_ENDIAN 4321
#define __PDP_ENDIAN 3412
#define __BYTE_ORDER __LITTLE_ENDIAN
#endif