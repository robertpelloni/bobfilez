/// @file data_pruner.cpp
/// @brief Implementation of the Digital Rot detection logic.

#include "fo/core/data_pruner_interface.hpp"
#include "fo/core/registry.hpp"
#include <regex>
#include <iostream>
#include <algorithm>

namespace fo::core {

class DataPrunerImpl : public IDataPruner {
    double min_confidence_ = 0.5;

    struct Rule {
        std::string name;
        std::regex pattern;
        double weight;
    };
    std::vector<Rule> rules_ = {
        {"Obsolete Installer", std::regex(R"(.*(setup|install|v\d+\.\d+\.\d+).*\.(exe|msi|dmg|pkg|deb)$)", std::regex::icase), 0.8},
        {"Temporary File", std::regex(R"(.*(\.tmp|\.log|\.bak|\.old|\.cache)$)", std::regex::icase), 0.95},
        {"Incomplete Download", std::regex(R"(.*(\.part|\.crdownload)$)", std::regex::icase), 1.0},
        {"Node Modules", std::regex(R"(.*node_modules.*)", std::regex::icase), 0.7},
        {"Large Media Cache", std::regex(R"(.*(Thumbnails|Cache|Unity|Local Storage).*)", std::regex::icase), 0.6}
    };

public:
    std::vector<PruningSuggestion> analyze(const std::filesystem::path& root) override {
        std::vector<PruningSuggestion> suggestions;
        if (!std::filesystem::exists(root)) return suggestions;

        auto now = std::chrono::system_clock::now();

        try {
            for (auto const& entry : std::filesystem::recursive_directory_iterator(root)) {
                if (!entry.is_regular_file()) continue;

                std::string fname = entry.path().filename().string();
                uintmax_t size = entry.file_size();
                auto mtime = entry.last_write_time();
                auto sys_mtime = std::chrono::clock_cast<std::chrono::system_clock>(mtime);
                
                double confidence = 0.0;
                std::string reason;

                // Match against rules
                for (const auto& rule : rules_) {
                    if (std::regex_match(fname, rule.pattern)) {
                        confidence = rule.weight;
                        reason = rule.name;
                        break;
                    }
                }

                // Age factor: if not touched in 1 year, boost confidence
                auto age = std::chrono::duration_cast<std::chrono::hours>(now - sys_mtime).count();
                if (age > 24 * 365) {
                    confidence += 0.2;
                    if (reason.empty()) reason = "Forgotten Data";
                }

                // Size factor: larger files are better pruning targets
                if (size > 500 * 1024 * 1024) { // > 500MB
                    confidence += 0.1;
                }

                if (confidence >= min_confidence_) {
                    PruningSuggestion s;
                    s.path = entry.path();
                    s.reason = reason;
                    s.confidence = std::clamp(confidence, 0.0, 1.0);
                    s.size = size;
                    s.safe_to_auto_delete = (confidence > 0.9);
                    suggestions.push_back(std::move(s));
                }
            }
        } catch (...) {}

        return suggestions;
    }

    void set_min_confidence(double score) override { min_confidence_ = score; }

    void add_custom_rule(const std::string& name, const std::string& pattern, double weight) override {
        try {
            rules_.push_back({name, std::regex(pattern, std::regex::icase), weight});
        } catch (...) {}
    }
};

static auto reg = []() {
    Registry<IDataPruner>::instance().add("default", []() {
        return std::make_unique<DataPrunerImpl>();
    });
    return true;
}();

} // namespace fo::core
