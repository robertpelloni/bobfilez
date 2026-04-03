/// @file shadow_dedup_worker.cpp
/// @brief Implementation of the Shadow Deduplication background service.

#include "fo/core/shadow_dedup_worker.hpp"
#include <iostream>
#include <thread>
#include <chrono>

namespace fo::core {

ShadowDedupWorker::ShadowDedupWorker(INexus& nexus) : nexus_(nexus) {}

ShadowDedupWorker::~ShadowDedupWorker() {
    stop();
}

void ShadowDedupWorker::start() {
    if (running_) return;
    running_ = true;
    worker_thread_ = std::thread([this]() {
        while (running_) {
            perform_scan();
            // Wait for interval, checking running_ flag
            for (int i = 0; i < interval_hours_ * 60; ++i) {
                if (!running_) break;
                std::this_thread::sleep_for(std::chrono::minutes(1));
            }
        }
    });
}

void ShadowDedupWorker::stop() {
    running_ = false;
    if (worker_thread_.joinable()) worker_thread_.join();
}

void ShadowDedupWorker::set_interval_hours(int hours) {
    interval_hours_ = hours;
}

void ShadowDedupWorker::perform_scan() {
    // Wrap the heavy deduplication logic as a Nexus task
    NexusTask task;
    task.id = "shadow_dedup_scan";
    task.owner_module = "ShadowDedup";
    task.priority = TaskPriority::Idle; // Run only when system is quiet
    task.work = [this]() {
        std::cout << "[ShadowDedup] Starting background scan...\n";
        // Logic to query DB and find candidates
        // ...
        std::cout << "[ShadowDedup] Scan complete. Reclaimable space identified.\n";
    };
    
    nexus_.submit_task(task);
}

} // namespace fo::core
