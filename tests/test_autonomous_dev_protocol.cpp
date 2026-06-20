#include <gtest/gtest.h>
#include "fo/core/autonomous_dev_protocol_interface.hpp"
#include "fo/core/swarm_engine_interface.hpp"
#include "fo/core/nexus_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

using namespace fo::core;

class AutonomousDevProtocolTest : public ::testing::Test {
protected:
    std::filesystem::path test_repo = "test_autodev_repo";

    void SetUp() override {
        register_all_providers();
        std::filesystem::create_directories(test_repo);
    }

    void TearDown() override {
        Registry<IAutonomousDevProtocol>::instance().clear_shared_instances();
        Registry<ISwarmEngine>::instance().clear_shared_instances();
        Registry<INexus>::instance().clear_shared_instances();
        std::filesystem::remove_all(test_repo);
    }
};

TEST_F(AutonomousDevProtocolTest, TaskRegistrationAndManualTrigger) {
    auto autodev = Registry<IAutonomousDevProtocol>::instance().get_shared("default");
    ASSERT_NE(autodev, nullptr);

    DevTask task;
    task.id = "build-01";
    task.name = "Initial Build";
    task.type = DevTaskType::Build;
    task.command = "simulate";
    
    autodev->register_task(task);
    autodev->trigger_task("build-01");

    auto status = autodev->get_dev_status();
    EXPECT_EQ(status.last_results["build-01"], "Success");
}

TEST_F(AutonomousDevProtocolTest, TaskDependencyHandling) {
    auto autodev = Registry<IAutonomousDevProtocol>::instance().get_shared("default");
    
    DevTask t1;
    t1.id = "t1"; t1.name = "Parent"; t1.type = DevTaskType::Build;
    t1.command = "simulate";
    autodev->register_task(t1);

    DevTask t2;
    t2.id = "t2"; t2.name = "Child"; t2.type = DevTaskType::Test;
    t2.command = "simulate";
    t2.dependencies = {"t1"};
    autodev->register_task(t2);

    // Trigger child first (should be blocked)
    autodev->trigger_task("t2");
    auto status = autodev->get_dev_status();
    EXPECT_TRUE(status.last_results["t2"].empty());

    // Trigger parent then child
    autodev->trigger_task("t1");
    autodev->trigger_task("t2");
    status = autodev->get_dev_status();
    EXPECT_EQ(status.last_results["t1"], "Success");
    EXPECT_EQ(status.last_results["t2"], "Success");
}

TEST_F(AutonomousDevProtocolTest, AutomatedLoopTrigger) {
    auto autodev = Registry<IAutonomousDevProtocol>::instance().get_shared("default");
    
    DevTask build;
    build.id = "build-primary"; build.name = "Build"; build.type = DevTaskType::Build;
    build.command = "simulate";
    autodev->register_task(build);

    DevTask test;
    test.id = "test-primary"; test.name = "Test"; test.type = DevTaskType::Test;
    test.command = "simulate";
    autodev->register_task(test);

    autodev->start_autodev_loop(test_repo);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Simulate file change
    {
        std::ofstream ofs(test_repo / "main.cpp");
        ofs << "int main() { return 0; }";
        ofs.flush();
    }

    // Wait for debounce (500ms) + processing
    for (int i = 0; i < 20; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        auto status = autodev->get_dev_status();
        if (status.last_results["build-primary"] == "Success" && 
            status.last_results["test-primary"] == "Success") break;
    }

    auto status = autodev->get_dev_status();
    EXPECT_EQ(status.last_results["build-primary"], "Success");
    EXPECT_EQ(status.last_results["test-primary"], "Success");

    autodev->stop_autodev_loop();
}
