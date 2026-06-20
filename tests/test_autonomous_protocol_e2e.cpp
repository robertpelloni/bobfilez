#include <gtest/gtest.h>
#include "fo/core/engine.hpp"
#include "fo/core/omniflow_engine_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

using namespace fo::core;

class AutonomousProtocolE2ETest : public ::testing::Test {
protected:
    std::filesystem::path test_base = "test_e2e_autonomous";
    std::filesystem::path inbox = test_base / "inbox";
    std::filesystem::path outbox = test_base / "outbox";
    std::filesystem::path db_path = test_base / "test.db";

    void SetUp() override {
        register_all_providers();
        std::filesystem::create_directories(inbox);
        std::filesystem::create_directories(outbox);
    }

    void TearDown() override {
        Registry<IOmniFlowEngine>::instance().clear_shared_instances();
        std::filesystem::remove_all(test_base);
    }
};

TEST_F(AutonomousProtocolE2ETest, FolderWatcherToMoveWorkflow) {
    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto& flow = engine.omniflow_engine();

    // 1. Define a workflow: Watch inbox -> Filter .txt -> Move to outbox
    Workflow wf;
    wf.id = "e2e-flow";
    wf.name = "E2E Autonomous Move";
    wf.is_active = true;

    wf.nodes.push_back({"trig1", "Trigger.FolderWatcher", FlowNodeType::Trigger, {{"path", inbox.string()}}});
    wf.nodes.push_back({"filt1", "Filter.Extension", FlowNodeType::Filter, {{"ext", ".txt"}}});
    wf.nodes.push_back({"act1", "Action.MoveTo", FlowNodeType::Action, {{"target_dir", outbox.string()}}});

    wf.connections.push_back({"trig1", "out", "filt1", "in"});
    wf.connections.push_back({"filt1", "true", "act1", "in"});

    flow.register_workflow(wf);

    // 2. Start the automation daemon
    flow.start_daemon();
    
    // Give some time for watcher thread to spin up
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 3. Create a file in the inbox
    std::filesystem::path test_file = inbox / "test_doc.txt";
    {
        std::ofstream ofs(test_file);
        ofs << "Autonomous processing content";
        ofs.flush();
    }

    // 4. Wait for processing (debounce 1s + logic)
    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    // 5. Verify the file moved to outbox
    EXPECT_FALSE(std::filesystem::exists(test_file));
    EXPECT_TRUE(std::filesystem::exists(outbox / "test_doc.txt"));
}
