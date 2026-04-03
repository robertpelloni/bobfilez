/// @file auto_cleanup_manager.cpp
/// @brief Implementation of the Auto-Cleanup service.

#include "fo/core/auto_cleanup_manager.hpp"
#include <iostream>

namespace fo::core {

AutoCleanupManager::AutoCleanupManager(std::shared_ptr<IDataPruner> pruner,
                                       std::shared_ptr<EnhancedCopyEngine> copy_engine,
                                       INexus& nexus)
    : pruner_(pruner), copy_engine_(copy_engine), nexus_(nexus) {}

void AutoCleanupManager::run_smart_purge(const std::filesystem::path& root, double min_confidence) {
    NexusTask task;
    task.id = "auto_cleanup_purge";
    task.owner_module = "AutoCleanup";
    task.priority = TaskPriority::Low;
    task.work = [this, root, min_confidence]() {
        auto targets = pruner_->analyze(root);
        std::vector<std::filesystem::path> to_delete;
        for (const auto& s : targets) {
            if (s.confidence >= min_confidence) {
                to_delete.push_back(s.path);
            }
        }
        
        if (!to_delete.empty()) {
            std::cout << "[AutoCleanup] Purging " << to_delete.size() << " files...\n";
            // Logic to perform secure delete via copy_engine
        }
    };
    
    nexus_.submit_task(task);
}

void AutoCleanupManager::schedule_maintenance(int interval_days) {
    // Schedule via Nexus timers
}

} // namespace fo::core
