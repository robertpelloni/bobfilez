#pragma once
/// @file shadow_sorter_service.hpp
/// @brief Automated "Auto-Organize" service for bobfilez.
///
/// Watches specified "Input" directories (e.g. ~/Downloads) and applies
/// RuleEngine patterns to automatically move incoming files to their 
/// "Permanent Home" in real-time.

#include "fo/core/file_watcher_interface.hpp"
#include "fo/core/rule_engine.hpp"
#include "fo/core/enhanced_fileops_interface.hpp"
#include <memory>

namespace fo::core {

class ShadowSorterService {
public:
    ShadowSorterService(std::shared_ptr<IFileWatcher> watcher,
                        std::shared_ptr<RuleEngine> rules,
                        std::shared_ptr<EnhancedCopyEngine> copy_engine);

    /// Start the real-time automation
    void start(const std::vector<std::filesystem::path>& watch_paths);

    /// Stop the automation
    void stop();

private:
    void handle_file_events(const std::vector<FileChangeEvent>& events);

    std::shared_ptr<IFileWatcher> watcher_;
    std::shared_ptr<RuleEngine> rules_;
    std::shared_ptr<EnhancedCopyEngine> copy_engine_;
};

} // namespace fo::core
