/// @file treemap_engine.cpp
/// @brief Implementation of the Treemap generator and layout algorithm.

#include "fo/core/treemap_engine_interface.hpp"
#include "fo/core/registry.hpp"
#include <numeric>
#include <algorithm>

namespace fo::core {

class TreemapEngineImpl : public ITreemapEngine {
public:
    TreemapNode build_tree(const std::filesystem::path& root, int max_depth) override {
        TreemapNode node;
        node.name = root.filename().string();
        node.path = root;
        node.size = 0;

        try {
            if (std::filesystem::is_directory(root)) {
                if (max_depth > 0) {
                    for (auto const& entry : std::filesystem::directory_iterator(root)) {
                        if (entry.is_directory()) {
                            auto child = build_tree(entry.path(), max_depth - 1);
                            node.size += child.size;
                            node.children.push_back(std::move(child));
                        } else {
                            node.size += entry.file_size();
                        }
                    }
                } else {
                    // Just count size at max depth
                    for (auto const& entry : std::filesystem::recursive_directory_iterator(root)) {
                        if (entry.is_regular_file()) node.size += entry.file_size();
                    }
                }
            } else {
                node.size = std::filesystem::file_size(root);
            }
        } catch (...) {}

        // Sort children by size descending
        std::sort(node.children.begin(), node.children.end(), [](const auto& a, const auto& b) {
            return a.size > b.size;
        });

        return node;
    }

    void calculate_layout(TreemapNode& root, double x, double y, double width, double height) override {
        root.x = x; root.y = y; root.width = width; root.height = height;

        if (root.children.empty() || width <= 0 || height <= 0) return;

        // Simple slice-and-dice layout for now (Squarified is more complex)
        bool horizontal = width > height;
        double current_offset = horizontal ? x : y;
        uintmax_t total_child_size = 0;
        for (const auto& c : root.children) total_child_size += c.size;

        if (total_child_size == 0) return;

        for (auto& child : root.children) {
            double ratio = static_cast<double>(child.size) / total_child_size;
            if (horizontal) {
                double child_w = width * ratio;
                calculate_layout(child, current_offset, y, child_w, height);
                current_offset += child_w;
            } else {
                double child_h = height * ratio;
                calculate_layout(child, x, current_offset, width, child_h);
                current_offset += child_h;
            }
        }
    }
};

static auto reg = []() {
    Registry<ITreemapEngine>::instance().add("default", []() {
        return std::make_unique<TreemapEngineImpl>();
    });
    return true;
}();

} // namespace fo::core
