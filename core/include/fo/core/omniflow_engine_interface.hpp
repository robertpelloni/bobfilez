#pragma once
/// @file omniflow_engine_interface.hpp
/// @brief Visual Node-Based Automation Engine for bobfilez.
///
/// Executes user-defined workflows composed of Triggers, Filters, Conditions,
/// and Actions. Ties together all bobfilez engines (FileOps, AI, Converters, Vault).

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>
#include <functional>

namespace fo::core {

enum class FlowNodeType {
    Trigger,    // e.g., "On File Added", "On Schedule", "Manual"
    Filter,     // e.g., "Is Image", "Size > 5MB"
    Condition,  // e.g., "Contains Text", "AI Detects 'Invoice'"
    Action      // e.g., "Move File", "Convert to WebP", "Lock in Vault", "Notify"
};

struct FlowNode {
    std::string id;
    std::string type_name;      // e.g., "Action.MoveToVault"
    FlowNodeType category;
    std::map<std::string, std::string> config; // e.g., {"target_dir": "/Vault"}
};

struct FlowConnection {
    std::string from_node_id;
    std::string from_pin;       // e.g., "out", "true", "false"
    std::string to_node_id;
    std::string to_pin;         // e.g., "in"
};

struct Workflow {
    std::string id;
    std::string name;
    bool is_active;
    std::vector<FlowNode> nodes;
    std::vector<FlowConnection> connections;
};

class IOmniFlowEngine {
public:
    virtual ~IOmniFlowEngine() = default;

    /// Register a new workflow into the active engine
    virtual void register_workflow(const Workflow& workflow) = 0;

    /// Remove a workflow by ID
    virtual bool remove_workflow(const std::string& workflow_id) = 0;

    /// Manually trigger a workflow
    virtual bool execute_workflow(const std::string& workflow_id, const std::filesystem::path& input_payload) = 0;

    /// Get all registered workflows
    virtual std::vector<Workflow> get_workflows() = 0;

    /// Save all workflows to SQLite database
    virtual bool save_workflows(const std::filesystem::path& db_path) = 0;

    /// Load workflows from SQLite database (merges with in-memory)
    virtual bool load_workflows(const std::filesystem::path& db_path) = 0;

    /// Start the background daemon that listens for triggers (File Watcher, Timers)
    virtual void start_daemon() = 0;
};

} // namespace fo::core
