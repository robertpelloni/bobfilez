/// @file nexus.cpp
/// @brief Implementation of the Nexus Master Clock and Scheduler.
///
/// Implements priority-based task scheduling and resource arbitration to
/// prevent bobfilez from saturating system resources during heavy batch
/// operations (conversions, indexing, moves).

#include "fo/core/nexus_interface.hpp"
#include "fo/core/registry.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <iostream>

namespace fo::core {

class NexusImpl : public INexus {
    struct PriorityTask {
        NexusTask task;
        uint64_t arrival_tick;
        bool operator<(const PriorityTask& other) const {
            if (task.priority != other.task.priority)
                return task.priority < other.task.priority;
            return arrival_tick > other.arrival_tick; // FIFO within priority
        }
    };

    std::priority_queue<PriorityTask> task_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::vector<std::thread> workers_;
    std::atomic<bool> running_{false};
    std::atomic<uint64_t> tick_count_{0};

    // Resource management
    std::map<ResourceType, std::string> active_locks_;
    std::mutex resource_mutex_;

    // Performance tracking
    std::map<std::string, double> metrics_;
    mutable std::mutex metrics_mutex_;

    void worker_loop() {
        while (running_) {
            NexusTask current_task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                queue_cv_.wait(lock, [this] { return !task_queue_.empty() || !running_; });
                if (!running_) break;
                current_task = std::move(task_queue_.top().task);
                task_queue_.pop();
            }

            try {
                if (current_task.work) current_task.work();
                if (current_task.on_complete) current_task.on_complete(true);
            } catch (...) {
                if (current_task.on_complete) current_task.on_complete(false);
            }
        }
    }

public:
    NexusImpl() {
        running_ = true;
        unsigned int threads = std::thread::hardware_concurrency();
        if (threads == 0) threads = 4;
        for (unsigned int i = 0; i < threads; ++i) {
            workers_.emplace_back(&NexusImpl::worker_loop, this);
        }
    }

    ~NexusImpl() override {
        running_ = false;
        queue_cv_.notify_all();
        for (auto& w : workers_) if (w.joinable()) w.join();
    }

    void submit_task(const NexusTask& task) override {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        task_queue_.push({task, ++tick_count_});
        queue_cv_.notify_one();
    }

    bool acquire_resource(ResourceType type, const std::string& requester_id, bool blocking) override {
        std::unique_lock<std::mutex> lock(resource_mutex_);
        // Simple mutual exclusion for now
        if (active_locks_.count(type)) {
            if (!blocking) return false;
            // Simplified blocking logic: poll (Real impl would use CV per resource)
            while (active_locks_.count(type)) {
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                lock.lock();
            }
        }
        active_locks_[type] = requester_id;
        return true;
    }

    void release_resource(ResourceType type, const std::string& requester_id) override {
        std::lock_guard<std::mutex> lock(resource_mutex_);
        if (active_locks_[type] == requester_id) {
            active_locks_.erase(type);
        }
    }

    std::chrono::system_clock::time_point now() const override {
        return std::chrono::system_clock::now();
    }

    uint64_t logical_tick() override {
        return ++tick_count_;
    }

    void report_metric(const std::string& module, const std::string& name, double value) override {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        metrics_[module + "." + name] = value;
    }

    std::map<std::string, double> get_performance_snapshot() const override {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        return metrics_;
    }

    // ── Distributed Nexus (Node-to-Node) ────────────────────────────────────
    
    void join_swarm(const std::string& remote_host) {
        // Uses OmniNet::MeshNode to discover other bobfilez instances
        std::cout << "[Nexus] Joining data swarm: " << remote_host << "\n";
    }

    void broadcast_event(const std::string& type, const std::string& data) {
        // Logic to sync audit ledger entries across nodes
    }
};

// Register the Nexus
static auto reg = []() {
    Registry<INexus>::instance().add("default", []() {
        return std::make_unique<NexusImpl>();
    });
    return true;
}();

} // namespace fo::core
