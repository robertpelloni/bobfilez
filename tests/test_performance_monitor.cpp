#include <gtest/gtest.h>
#include "fo/core/engine.hpp"
#include "fo/core/nexus_interface.hpp"
#include "fo/core/registry.hpp"
#include <filesystem>

using namespace fo::core;

class PerformanceMonitorTest : public ::testing::Test {
protected:
    void SetUp() override {
        db_path = "perf_test.db";
    }
    void TearDown() override {
        if (std::filesystem::exists(db_path)) std::filesystem::remove(db_path);
    }
    std::filesystem::path db_path;
};

TEST_F(PerformanceMonitorTest, AggregatedHealthReport) {
    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto& nexus = engine.nexus();

    // Simulate metric reporting from different modules
    nexus.report_metric("OmniFlow", "ExecutionCount", 42.0);
    nexus.report_metric("OmniFlow", "LastExecutionDuration", 1.25);
    nexus.report_metric("SyncService", "UploadCount", 10.0);
    nexus.report_metric("SyncService", "LiveValidationSuccess", 1.0);

    auto health = engine.get_autonomous_health();

    EXPECT_EQ(health.total_workflows_executed, 42);
    EXPECT_DOUBLE_EQ(health.avg_workflow_duration, 1.25);
    EXPECT_EQ(health.total_sync_uploads, 10);
    EXPECT_TRUE(health.last_live_validation_success);
}
