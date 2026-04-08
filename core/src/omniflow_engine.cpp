/// @file omniflow_engine.cpp
/// @brief Implementation of the Visual Automation Engine.

#include "fo/core/omniflow_engine_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/nexus_interface.hpp"
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>

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
        std::cout << "[OmniFlow] Registered Workflow: " << workflow.name << " with " << workflow.nodes.size() << " nodes.\n";
    }

    bool execute_workflow(const std::string& workflow_id, const std::filesystem::path& input_payload) override {
        if (!workflows_.count(workflow_id)) return false;
        
        auto& wf = workflows_[workflow_id];
        std::cout << "[OmniFlow] Executing workflow: " << wf.name << " on payload: " << input_payload << "\n";
        
        // Example execution logic (Placeholder for AST node traversal)
        // 1. Find trigger node
        // 2. Follow connections
        // 3. Evaluate filters & conditions
        // 4. Dispatch actions to Nexus
        
        return true;
    }

    std::vector<Workflow> get_workflows() override {
        std::vector<Workflow> res;
        for (auto const& [id, wf] : workflows_) res.push_back(wf);
        return res;
    }

    void start_daemon() override {
        if (daemon_running_) return;
        daemon_running_ = true;
        std::cout << "[OmniFlow] Starting background trigger daemon...\n";
        
        // This daemon would normally attach to the FileWatcher and MasterClock.
        // E.g., if a file lands in "Downloads", it triggers the appropriate flow.
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
