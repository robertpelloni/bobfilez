#include <gtest/gtest.h>
#include "fo/core/autonomous_dev_protocol_interface.hpp"
#include "fo/core/autonomous_sync_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>
#include <thread>

using namespace fo::core;

class AutonomousDevComprehensiveTest : public ::testing::Test {
protected:
    std::filesystem::path repo = "comprehensive_repo";

    void SetUp() override {
        register_all_providers();
        std::filesystem::create_directories(repo);
    }

    void TearDown() override {
        Registry<IAutonomousDevProtocol>::instance().clear_shared_instances();
        Registry<IAutonomousSyncService>::instance().clear_shared_instances();
        std::filesystem::remove_all(repo);
    }
};

TEST_F(AutonomousDevComprehensiveTest, FeedbackAndApprovalLoop) {
    auto autodev = Registry<IAutonomousDevProtocol>::instance().get_shared("default");
    
    DevTask t;
    t.id = "feedback-test";
    t.name = "Feedback Task";
    t.command = "simulate";
    autodev->register_task(t);

    // Initial Trigger
    autodev->trigger_task("feedback-test");
    EXPECT_EQ(autodev->get_dev_status().last_results["feedback-test"], "Success");

    // Rejection Feedback
    autodev->provide_feedback("feedback-test", false, "logic error detected");
    EXPECT_EQ(autodev->get_dev_status().last_results["feedback-test"], "Rejected: logic error detected");

    // Approval Feedback
    autodev->provide_feedback("feedback-test", true, "fixed");
    EXPECT_EQ(autodev->get_dev_status().last_results["feedback-test"], "Success (Manual)");
}

TEST_F(AutonomousDevComprehensiveTest, CheckpointIntegration) {
    auto autodev = Registry<IAutonomousDevProtocol>::instance().get_shared("default");
    auto sync = Registry<IAutonomousSyncService>::instance().get_shared("default");
    sync->set_storage_path(repo / "sync.db");
    autodev->set_sync_service(sync);

    DevTask t;
    t.id = "checkpoint-test";
    t.name = "Checkpoint Task";
    t.command = "simulate";
    autodev->register_task(t);

    // Must set repo_root for checkpointing to trigger
    autodev->start_autodev_loop(repo);

    autodev->trigger_task("checkpoint-test");
    
    auto status = autodev->get_dev_status();
    EXPECT_EQ(status.last_checkpoint_id, "checkpoint-checkpoint-test");

    // Verify sync history contains the checkpoint
    std::cout << "[Test] Waiting for history to appear...\n";
    auto history = sync->get_version_history("checkpoints/checkpoint-test/pre_run");
    EXPECT_GE(history.size(), 1);

    autodev->stop_autodev_loop();
}
