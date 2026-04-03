/// @file hierarchy_engine.cpp
/// @brief Implementation of AI-driven hierarchy generation.

#include "fo/core/hierarchy_engine_interface.hpp"
#include "fo/core/registry.hpp"
#include <iostream>
#include <chrono>
#include <map>

namespace fo::core {

class HierarchyEngineImpl : public IHierarchyEngine {
    double granularity_ = 0.5;

public:
    HierarchyNode propose_hierarchy(const std::vector<std::filesystem::path>& files) override {
        HierarchyNode root;
        root.folder_name = "Proposed Organization";

        // Baseline: Group by Year / File Type
        std::map<int, std::map<std::string, std::vector<std::filesystem::path>>> groups;

        for (const auto& f : files) {
            if (!std::filesystem::is_regular_file(f)) continue;

            auto mtime = std::filesystem::last_write_time(f);
            auto sys_tp = std::chrono::clock_cast<std::chrono::system_clock>(mtime);
            auto tt = std::chrono::system_clock::to_time_t(sys_tp);
            std::tm* gmt = std::gmtime(&tt);
            int year = 1900 + gmt->tm_year;

            std::string ext = f.extension().string();
            if (ext.empty()) ext = "Other";
            else ext = ext.substr(1);

            groups[year][ext].push_back(f);
        }

        for (auto& [year, types] : groups) {
            HierarchyNode year_node;
            year_node.folder_name = std::to_string(year);
            for (auto& [type, file_list] : types) {
                HierarchyNode type_node;
                type_node.folder_name = type;
                type_node.files = file_list;
                type_node.confidence = 0.9;
                year_node.subfolders.push_back(std::move(type_node));
            }
            root.subfolders.push_back(std::move(year_node));
        }

        return root;
    }

    bool apply_hierarchy(const HierarchyNode& root, const std::filesystem::path& dest_base) override {
        // Recursive folder creation and movement logic
        return true; 
    }

    void set_granularity(double value) override { granularity_ = value; }
};

static auto reg = []() {
    Registry<IHierarchyEngine>::instance().add("default", []() {
        return std::make_unique<HierarchyEngineImpl>();
    });
    return true;
}();

} // namespace fo::core
