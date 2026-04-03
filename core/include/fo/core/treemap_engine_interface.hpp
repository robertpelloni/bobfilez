#pragma once
/// @file treemap_engine_interface.hpp
/// @brief High-performance disk usage tree-map generator for bobfilez.
///
/// Implements the Squarified Treemap algorithm to visualize folder sizes.
/// Calculates recursive directory sizes and categorizes files by type
/// for the TopologyPanel.

#include <string>
#include <vector>
#include <filesystem>

namespace fo::core {

struct TreemapNode {
    std::string name;
    std::filesystem::path path;
    uintmax_t size;
    std::string type; // "Video", "Image", "System", etc.
    std::vector<TreemapNode> children;
    
    // UI layout coordinates (calculated by engine)
    double x = 0, y = 0, width = 0, height = 0;
};

class ITreemapEngine {
public:
    virtual ~ITreemapEngine() = default;

    /// Recursively calculate sizes for a directory tree
    virtual TreemapNode build_tree(const std::filesystem::path& root, int max_depth = 4) = 0;

    /// Calculate layout coordinates (Squarified Treemap)
    virtual void calculate_layout(TreemapNode& root, double x, double y, double width, double height) = 0;
};

} // namespace fo::core
