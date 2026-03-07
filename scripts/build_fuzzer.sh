#!/bin/bash
set -e

cd "$(dirname "$0")/.."

echo "[INFO] Running CMake Configure for Fuzzers..."
export CC=clang
export CXX=clang++

TOOLCHAIN_ARGS=""
if [ -f "vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
    TOOLCHAIN_ARGS="-DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake"
fi

cmake -S . -B build_fuzz -G Ninja -DCMAKE_BUILD_TYPE=Release $TOOLCHAIN_ARGS -DFO_BUILD_GUI=OFF -DFO_BUILD_CLI=OFF -DFO_BUILD_BENCH=OFF -DFO_BUILD_TESTS=OFF -DFO_BUILD_FUZZERS=ON

echo "[INFO] Running CMake Build..."
cmake --build build_fuzz

echo "[SUCCESS] Fuzzer build complete."
