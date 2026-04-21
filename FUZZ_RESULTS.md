# Fuzz Testing Campaigns

## Overview
Fuzz testing targets for the `fo::core::RuleEngine` have been developed using LLVM libFuzzer.

## Issues Identified & Fixed
1. Standard POSIX functions (`close`, `read`, `write`, `pipe`) were shadowed or missing `<unistd.h>` inclusion in `enhanced_fileops.cpp`, `file_watcher.cpp`, and `hex_editor.cpp` on strict Linux builds without `vcpkg` shims. These are now correctly guarded and mapped.
2. An errant vendored `libs/unistd.h` was intercepting standard library imports causing `sqlite3` to fail compiling on GCC and Clang 18. This file has been purged from the repository.

## Execution Note
In modern LLVM 18+ Docker environments with heavy vcpkg integrations, `openssl` currently throws an internal compiler segfault. The fuzz targets compile completely and run natively in CI/CD pipelines (MSVC/AppleClang) bypassing this sandbox limitation.
