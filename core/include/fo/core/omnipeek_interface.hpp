#pragma once
/// @file omnipeek_interface.hpp
/// @brief Universal Instant Quick Look Engine for bobfilez.
///
/// Analyzes a file in milliseconds and determines the optimal way to render
/// it in the UI overlay. Bridges raylib (3D), mpv (Media), radare2 (Binary),
/// sqlite3 (Databases), and libarchive (Archives).

#include <string>
#include <vector>
#include <filesystem>
#include <map>
#include <memory>

namespace fo::core {

enum class PeekType {
    Image,
    Video,
    Audio,
    TextCode,
    ArchiveTree,
    DatabaseSchema,
    BinaryAssembly,
    Model3D,
    Unknown
};

struct PeekData {
    std::filesystem::path file_path;
    PeekType type;
    std::string primary_content; // Text, JSON schema, Assembly snippet, or URI to media
    std::map<std::string, std::string> metadata; // e.g. {"Resolution": "4K", "FPS": "60"}
    double load_time_ms;
};

class IOmniPeekEngine {
public:
    virtual ~IOmniPeekEngine() = default;

    /// Analyze a file and generate the payload for the QML UI to render
    virtual PeekData generate_preview(const std::filesystem::path& file) = 0;

    /// Pre-warm the engines (mpv, raylib, radare2) in the background
    virtual void prewarm_engines() = 0;
};

} // namespace fo::core
