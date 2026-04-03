#pragma once
/// @file omniverse_interface.hpp
/// @brief 3D Spatial File Explorer Engine for bobfilez.
///
/// OmniVerse uses raylib to render the filesystem as a 3D navigable space.
/// Calculates 3D coordinates (Force-Directed Graph) for files and folders,
/// turning the OS into a spatial computing environment.

#include <string>
#include <vector>
#include <filesystem>
#include <memory>

namespace fo::core {

struct Vector3D {
    float x, y, z;
};

enum class SpatialType {
    FolderNode,
    FileImage,
    FileVideo,
    FileCode,
    FileDoc,
    FileArchive,
    FileUnknown
};

struct SpatialNode {
    std::string id;
    std::string name;
    std::filesystem::path path;
    SpatialType type;
    Vector3D position;
    float radius;
    std::string color_hex;
};

struct SpatialLink {
    std::string source_id;
    std::string target_id;
};

class IOmniVerseEngine {
public:
    virtual ~IOmniVerseEngine() = default;

    /// Launch the hardware-accelerated raylib 3D window
    virtual bool launch_3d_viewport(const std::filesystem::path& root_dir) = 0;

    /// Calculate the 3D coordinate mapping for a directory structure
    virtual std::pair<std::vector<SpatialNode>, std::vector<SpatialLink>> map_directory(const std::filesystem::path& root) = 0;

    /// Check if OpenGL/raylib context is available
    virtual bool is_hardware_accelerated() const = 0;
};

} // namespace fo::core
