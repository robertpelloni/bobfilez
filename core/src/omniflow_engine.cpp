/// @file omniflow_engine.cpp
/// @brief Implementation of the Visual Automation Engine.

#include "fo/core/omniflow_engine_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/nexus_interface.hpp"
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <set>
#include <filesystem>

namespace fo::core {

class OmniFlowEngineImpl : public IOmniFlowEngine {
    std::map<std::string, Workflow> workflows_;
    bool daemon_running_ = false;

public:
    OmniFlowEngineImpl() {
        // Preload an example workflow for the UI preview
        Workflow w1;
        w1.id = "flow-01";
        w1.name = "Auto-Vault Financials";
        w1.is_active = true;

        w1.nodes.push_back({"node1", "Trigger.FolderWatcher", FlowNodeType::Trigger, {{"path", "~/Downloads"}}});
        w1.nodes.push_back({"node2", "Filter.IsPDF", FlowNodeType::Filter, {}});
        w1.nodes.push_back({"node3", "Condition.TextContains", FlowNodeType::Condition, {{"text", "Invoice"}}});
        w1.nodes.push_back({"node4", "Action.MoveToVault", FlowNodeType::Action, {{"vault_id", "secure_1"}}});
        w1.nodes.push_back({"node5", "Action.NotifyUser", FlowNodeType::Action, {{"msg", "Invoice secured!"}}});

        w1.connections.push_back({"node1", "out", "node2", "in"});
        w1.connections.push_back({"node2", "true", "node3", "in"});
        w1.connections.push_back({"node3", "true", "node4", "in"});
        w1.connections.push_back({"node4", "out", "node5", "in"});
        
        workflows_[w1.id] = w1;
    }

    void register_workflow(const Workflow& workflow) override {
        workflows_[workflow.id] = workflow;
    }

    /// Find the next nodes reachable from a given node via any connection
    std::vector<std::string> find_next_nodes(const Workflow& wf, const std::string& node_id, const std::string& pin = "") {
        std::vector<std::string> next;
        for (const auto& conn : wf.connections) {
            if (conn.from_node_id == node_id) {
                if (pin.empty() || conn.from_pin == pin || conn.from_pin == "out") {
                    next.push_back(conn.to_node_id);
                }
            }
        }
        return next;
    }

    /// Find a node by ID within a workflow
    const FlowNode* find_node(const Workflow& wf, const std::string& node_id) {
        for (const auto& n : wf.nodes) {
            if (n.id == node_id) return &n;
        }
        return nullptr;
    }

    /// Evaluate a single filter/condition node against a payload file
    bool evaluate_node(const FlowNode& node, const std::filesystem::path& payload) {
        if (node.category == FlowNodeType::Filter) {
            // Built-in filters
            if (node.type_name == "Filter.IsPDF") {
                return payload.has_extension() && (payload.extension() == ".pdf" || payload.extension() == ".PDF");
            }
            if (node.type_name == "Filter.IsImage") {
                auto ext = payload.extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                return ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".gif" || ext == ".bmp" || ext == ".webp";
            }
            if (node.type_name == "Filter.Extension") {
                auto it = node.config.find("ext");
                if (it != node.config.end()) {
                    return payload.has_extension() && payload.extension() == it->second;
                }
            }
            if (node.type_name == "Filter.IsLarger") {
                auto it = node.config.find("min_bytes");
                if (it != node.config.end()) {
                    try {
                        auto size = std::filesystem::file_size(payload);
                        return size >= std::stoull(it->second);
                    } catch (...) { return false; }
                }
            }
        }

        if (node.category == FlowNodeType::Condition) {
            if (node.type_name == "Condition.TextContains") {
                // Check filename contains text
                auto it = node.config.find("text");
                if (it != node.config.end()) {
                    auto fname = payload.filename().string();
                    std::transform(fname.begin(), fname.end(), fname.begin(), ::tolower);
                    auto needle = it->second;
                    std::transform(needle.begin(), needle.end(), needle.begin(), ::tolower);
                    return fname.find(needle) != std::string::npos;
                }
            }
            if (node.type_name == "Condition.AlwaysTrue") {
                return true;
            }
        }

        // Default: allow passthrough
        return true;
    }

    /// Execute an action node against a payload file
    bool execute_action(const FlowNode& node, const std::filesystem::path& payload) {
        if (node.type_name == "Action.MoveTo") {
            auto it = node.config.find("target_dir");
            if (it != node.config.end() && std::filesystem::exists(payload)) {
                try {
                    auto dest = std::filesystem::path(it->second) / payload.filename();
                    std::filesystem::create_directories(it->second);
                    std::filesystem::rename(payload, dest);
                    return true;
                } catch (...) { return false; }
            }
        }
        if (node.type_name == "Action.CopyTo") {
            auto it = node.config.find("target_dir");
            if (it != node.config.end() && std::filesystem::exists(payload)) {
                try {
                    auto dest = std::filesystem::path(it->second) / payload.filename();
                    std::filesystem::create_directories(it->second);
                    std::filesystem::copy_file(payload, dest, std::filesystem::copy_options::overwrite_existing);
                    return true;
                } catch (...) { return false; }
            }
        }
        if (node.type_name == "Action.Delete") {
            try {
                return std::filesystem::remove(payload);
            } catch (...) { return false; }
        }
        // Actions that don't need file I/O just succeed
        return true;
    }

    bool execute_workflow(const std::string& workflow_id, const std::filesystem::path& input_payload) override {
        if (!workflows_.count(workflow_id)) return false;
        
        auto& wf = workflows_[workflow_id];

        // 1. Find all trigger nodes
        std::vector<std::string> trigger_ids;
        for (const auto& n : wf.nodes) {
            if (n.category == FlowNodeType::Trigger) {
                trigger_ids.push_back(n.id);
            }
        }
        if (trigger_ids.empty()) return false;

        // 2. BFS from each trigger, evaluating filters/conditions along the way
        std::set<std::string> visited;
        std::vector<std::string> queue;
        for (const auto& tid : trigger_ids) queue.push_back(tid);

        size_t front = 0;
        while (front < queue.size()) {
            std::string current_id = queue[front++];
            if (visited.count(current_id)) continue;
            visited.insert(current_id);

            const FlowNode* node = find_node(wf, current_id);
            if (!node) continue;

            if (node->category == FlowNodeType::Filter || node->category == FlowNodeType::Condition) {
                bool passed = evaluate_node(*node, input_payload);
                if (!passed) {
                    // Follow "false" pin if exists, otherwise stop this branch
                    auto false_next = find_next_nodes(wf, current_id, "false");
                    for (const auto& nid : false_next) queue.push_back(nid);
                    continue;
                }
                // Follow "true" or "out" pin
                auto true_next = find_next_nodes(wf, current_id, "true");
                if (true_next.empty()) true_next = find_next_nodes(wf, current_id, "out");
                for (const auto& nid : true_next) queue.push_back(nid);
            } else if (node->category == FlowNodeType::Action) {
                execute_action(*node, input_payload);
                // Continue to next nodes via "out" pin
                auto next = find_next_nodes(wf, current_id, "out");
                for (const auto& nid : next) queue.push_back(nid);
            } else {
                // Trigger nodes: just pass through
                auto next = find_next_nodes(wf, current_id, "out");
                for (const auto& nid : next) queue.push_back(nid);
            }
        }

        return true;
    }

    std::vector<Workflow> get_workflows() override {
        std::vector<Workflow> res;
        for (auto const& [id, wf] : workflows_) res.push_back(wf);
        return res;
    }

    void start_daemon() override {
        daemon_running_ = true;
    }
};

static bool register_omniflow_engine_guard = []() {
    Registry<IOmniFlowEngine>::instance().add("default", []() {
        return std::make_unique<OmniFlowEngineImpl>();
    });
    return true;
}();

void register_omniflow_engine() { (void)register_omniflow_engine_guard; }

} // namespace fo::core
