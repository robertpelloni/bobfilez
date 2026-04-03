#pragma once
/// @file auto_cleanup_manager.hpp
/// @brief Automated system maintenance for bobfilez.
///
/// Coordinates the DataPruner and EnhancedFileOps to perform periodic
/// cleanups of "Digital Rot". Can be scheduled or triggered via the Dashboard.

#include "fo/core/data_pruner_interface.hpp"
#include "fo/core/enhanced_fileops_interface.hpp"
#include "fo/core/nexus_interface.hpp"
#include <memory>

namespace fo::core {

class AutoCleanupManager {
public:
    AutoCleanupManager(std::shared_ptr<IDataPruner> pruner,
                       std::shared_ptr<EnhancedCopyEngine> copy_engine,
                       INexus& nexus);

    /// Perform a "Smart Purge" on the given root directory
    void run_smart_purge(const std::filesystem::path& root, double min_confidence = 0.9);

    /// Schedule periodic cleanups
    void schedule_maintenance(int interval_days);

private:
    std::shared_ptr<IDataPruner> pruner_;
    std::shared_ptr<EnhancedCopyEngine> copy_engine_;
    INexus& nexus_;
};

} // namespace fo::core
