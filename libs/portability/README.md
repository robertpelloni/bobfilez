# libs/portability — host shims for toolchains with incomplete C libraries (NOT a submodule)
endian.h: minimal glibc-style byte-order macros for MinGW-w64 GCC, required by
libs/hash-library (*.cpp take the POSIX branch via `#ifndef _MSC_VER`).
MSVC never reads it (takes the _MSC_VER branch); glibc hosts use their own.
Added 2026-11-03 during the portable-MinGW build lane.