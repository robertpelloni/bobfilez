#include "fo/core/engine.hpp"
#include <emscripten/bind.h>
#include <vector>
#include <string>

using namespace emscripten;
using namespace fo::core;

/// @file wasm_bridge.cpp
/// @brief Emscripten bindings for bobfilez core.
///
/// This allows the high-performance C++ engine to run in the browser
/// via WebAssembly, enabling "Universal Data Custodian" features.

EMSCRIPTEN_BINDINGS(bobfilez) {
    class_<FileInfo>("FileInfo")
        .constructor<>()
        .property("path", &FileInfo::uri)
        .property("size", &FileInfo::size);

    register_vector<FileInfo>("FileInfoList");

    class_<Engine>("Engine")
        .constructor<>()
        .function("scan", &Engine::scan);
}
