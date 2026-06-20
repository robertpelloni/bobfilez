#pragma once
/// @file autonomous_dev_protocol_interface.hpp
/// @brief Continuous Integration and Autonomous Development loop for bobfilez.
///
/// Implements a "Self-Developing" engine that watches the codebase,
/// detects architectural shifts, and automatically triggers builds, 
/// tests, and linting tasks.

#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include <functional>
#include <map>

namespace fo::core {

enum class DevTaskType {
    Build,
    Test,
    Lint,
    Document,
    Deploy
};

struct DevTask {
    std::string id;
    std::string name;
    DevTaskType type;
    std::string command;
    std::vector<std::string> dependencies; // Task IDs
};

struct DevStatus {
    bool is_loop_active;
    std::vector<std::string> active_tasks;
    std::map<std::string, std::string> last_results; // Task ID -> Result
    std::string last_checkpoint_id;
};

class IAutonomousSyncService;

class IAutonomousDevProtocol {
public:
    virtual ~IAutonomousDevProtocol() = default;

    /// Set the sync service for checkpointing before tasks
    virtual void set_sync_service(std::shared_ptr<IAutonomousSyncService> sync) = 0;

    /// Register a development task
    virtual void register_task(const DevTask& task) = 0;

    /// Trigger a specific task manually
    virtual void trigger_task(const std::string& task_id) = 0;

    /// Provide feedback (human-in-the-loop) for a task
    virtual void provide_feedback(const std::string& task_id, bool approved, const std::string& comment = "") = 0;

    /// Start the autonomous development loop (watching src/ and tests/)
    virtual void start_autodev_loop(const std::filesystem::path& repo_root) = 0;

    /// Stop the loop
    virtual void stop_autodev_loop() = 0;

    /// Get current development status
    virtual DevStatus get_dev_status() = 0;
};

} // namespace fo::core
