/// @file wasm_bridge.cpp
/// @brief Emscripten bindings for bobfilez core.
///
/// This translation unit is only relevant for WebAssembly builds.
/// Native desktop builds should compile successfully without any Emscripten SDK.

#ifdef __EMSCRIPTEN__

#include "fo/core/engine.hpp"
#include <emscripten/bind.h>
#include <string>
#include <vector>

using namespace emscripten;
using namespace fo::core;

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

#endif
