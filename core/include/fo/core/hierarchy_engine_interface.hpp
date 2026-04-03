#pragma once
/// @file hierarchy_engine_interface.hpp
/// @brief AI-driven automated folder structure generation for bobfilez.
///
/// Uses clustering algorithms (K-Means/DBSCAN) on file metadata and CLIP 
/// embeddings to propose a "Perfect Hierarchy". 
/// Examples:
///   - "/Photos/2024/January/Tahoe"
///   - "/Work/Projects/Project-Alpha/Specs"
///   - "/Music/FLAC/High-Res/Jazz"

#include <string>
#include <vector>
#include <filesystem>
#include <map>

namespace fo::core {

struct HierarchyNode {
    std::string folder_name;
    std::vector<std::filesystem::path> files;
    std::vector<HierarchyNode> subfolders;
    double confidence;
};

class IHierarchyEngine {
public:
    virtual ~IHierarchyEngine() = default;

    /// Analyze a flat list of files and propose a nested hierarchy
    virtual HierarchyNode propose_hierarchy(const std::vector<std::filesystem::path>& files) = 0;

    /// Commit the proposed hierarchy (actually move files)
    virtual bool apply_hierarchy(const HierarchyNode& root, const std::filesystem::path& dest_base) = 0;

    /// Set reorganization strictness (0.0 = few folders, 1.0 = many deep folders)
    virtual void set_granularity(double value) = 0;
};

} // namespace fo::core
