#!/bin/bash
export CC=gcc
export CXX=g++
rm -rf build_fuzz
cmake -S . -B build_fuzz -DFO_BUILD_FUZZERS=ON -DFO_BUILD_CLI=OFF -DFO_BUILD_OMNI=OFF -DFO_BUILD_GUI=OFF -DFO_BUILD_TESTS=OFF -DCMAKE_TOOLCHAIN_FILE="$PWD/vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build_fuzz --target fuzz_rule_engine
./build_fuzz/fuzz/fuzz_rule_engine -max_total_time=10
