#pragma once
/// @file nexus_interface.hpp
/// @brief Master Synchronization & Clock Unification for bobfilez ("The Nexus").
///
/// Nexus provides a single source of truth for time, event ordering, and 
/// cross-module coordination. It handles:
///   - High-precision logical clocks (Lamport/Vector) for distributed ops
///   - Task scheduling (priority-based)
///   - Resource locking (preventing I/O contention between engines)
///   - Real-time performance monitoring across all submodules
///
/// All major engines (Search, Rename, Convert, FileOps) register with Nexus
/// to coordinate heavy I/O and CPU tasks.

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <memory>
#include <map>

namespace fo::core {

/// Priority levels for the Nexus scheduler
enum class TaskPriority {
    Idle,           ///< Background indexing, maintenance
    Low,            ///< Large batch conversions
    Normal,         ///< Standard file operations
    High,           ///< UI-blocking search results
    RealTime        ///< Critical file-watcher events
};

/// A task descriptor for the Nexus scheduler
struct NexusTask {
    std::string id;
    std::string owner_module;
    TaskPriority priority = TaskPriority::Normal;
    std::function<void()> work;
    std::function<void(float)> on_progress;
    std::function<void(bool)> on_complete;
};

/// High-level resource types that Nexus arbitrates
enum class ResourceType {
    DiskIO,         ///< Prevents multiple engines from saturating disk bandwidth
    NetworkIO,      ///< Limits cloud provider throughput
    CpuHeavy,       ///< Limits concurrent ML inference / video transcoding
    Database        ///< SQLite write-lock management
};

/// The Nexus Interface
class INexus {
public:
    virtual ~INexus() = default;

    // ── Scheduling ───────────────────────────────────────────────────────
    
    /// Schedule a task for execution
    virtual void submit_task(const NexusTask& task) = 0;

    /// Request exclusive or shared access to a shared resource
    virtual bool acquire_resource(ResourceType type, const std::string& requester_id, bool blocking = true) = 0;
    
    /// Release an acquired resource
    virtual void release_resource(ResourceType type, const std::string& requester_id) = 0;

    // ── Time & Events ────────────────────────────────────────────────────

    /// Get synchronized system time
    virtual std::chrono::system_clock::time_point now() const = 0;

    /// Get current logical clock value (monotonically increasing)
    virtual uint64_t logical_tick() = 0;

    // ── Monitoring ───────────────────────────────────────────────────────

    /// Report performance metrics from a submodule
    virtual void report_metric(const std::string& module, const std::string& name, double value) = 0;

    /// Get a snapshot of current system-wide performance
    virtual std::map<std::string, double> get_performance_snapshot() const = 0;
};

} // namespace fo::core
