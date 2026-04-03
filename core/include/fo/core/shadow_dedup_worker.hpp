#pragma once
/// @file shadow_dedup_worker.hpp
/// @brief Background deduplication scanner ("Shadow Deduplication").
///
/// Runs at the lowest priority via Nexus. Periodically scans indexed folders
/// to find redundant data without user prompting. Emits notifications to the
/// shell when significant space can be reclaimed.

#include "fo/core/nexus_interface.hpp"
#include <string>
#include <vector>
#include <atomic>

namespace fo::core {

struct ShadowDedupResult {
    int group_count;
    uintmax_t potential_reclaim_bytes;
    std::string summary;
};

class ShadowDedupWorker {
public:
    ShadowDedupWorker(INexus& nexus);
    ~ShadowDedupWorker();

    /// Start background scanning
    void start();
    
    /// Stop background scanning
    void stop();

    /// Set scan interval (e.g. once every 24 hours)
    void set_interval_hours(int hours);

private:
    void perform_scan();

    INexus& nexus_;
    std::atomic<bool> running_{false};
    std::thread worker_thread_;
    int interval_hours_ = 24;
};

} // namespace fo::core
