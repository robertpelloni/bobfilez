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
                    std::error_code dec;
                    for (std::filesystem::directory_iterator it(root, dec), end;
                         it != end; it.increment(dec)) {
                        if (dec) { dec.clear(); break; }
                        std::error_code fec;
                        if (it->is_directory(fec) && !fec) {
                            auto child = build_tree(it->path(), max_depth - 1);
                            node.size += child.size;
                            node.children.push_back(std::move(child));
                        } else if (it->is_regular_file(fec) && !fec) {
                            node.size += it->file_size(fec);
                        }
                    }
                } else {
                    // Just count size at max depth. Manual stack
                    // DFS: the throwing recursive walk aborted
                    // the whole sizing on >260-char subtrees.
                    std::vector<std::filesystem::path> stack{root};
                    while (!stack.empty()) {
                        auto cur = stack.back();
                        stack.pop_back();
                        std::error_code dec;
                        for (std::filesystem::directory_iterator it(cur, dec), end;
                             it != end; it.increment(dec)) {
                            if (dec) { dec.clear(); break; }
                            std::error_code fec;
                            if (it->is_directory(fec) && !fec) {
                                if (!it->is_symlink(fec)) stack.push_back(it->path());
                            } else if (it->is_regular_file(fec) && !fec) {
                                auto sz = it->file_size(fec);
                                if (!fec) node.size += sz;
                            }
                        }
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

static bool reg_treemap = []() {
    Registry<ITreemapEngine>::instance().add("default", []() {
        return std::make_unique<TreemapEngineImpl>();
    });
    return true;
}();

void register_treemap_engine() { (void)reg_treemap; }

} // namespace fo::core
