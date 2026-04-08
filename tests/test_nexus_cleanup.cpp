#include <gtest/gtest.h>
#include "fo/core/nexus_interface.hpp"
#include "fo/core/auto_cleanup_manager.hpp"
#include "fo/core/data_pruner_interface.hpp"
#include "fo/core/enhanced_fileops_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <map>

using namespace fo::core;

// ── Mock Nexus for AutoCleanup Tests ─────────────────────────────────────

class MockNexus : public INexus {
    std::atomic<uint64_t> tick_{0};
    std::map<std::string, double> metrics_;
    mutable std::mutex metrics_mutex_;
    std::atomic<bool> resource_held_{false};

public:
    std::vector<NexusTask> submitted_tasks;
    std::mutex tasks_mutex;

    void submit_task(const NexusTask& task) override {
        std::lock_guard<std::mutex> lock(tasks_mutex);
        submitted_tasks.push_back(task);
        // Immediately execute for testing
        if (task.work) task.work();
    }

    bool acquire_resource(ResourceType /*type*/, const std::string& /*id*/, bool /*blocking*/) override {
        return !resource_held_.exchange(true);
    }

    void release_resource(ResourceType /*type*/, const std::string& /*id*/) override {
        resource_held_ = false;
    }

    std::chrono::system_clock::time_point now() const override {
        return std::chrono::system_clock::now();
    }

    uint64_t logical_tick() override { return ++tick_; }

    void report_metric(const std::string& module, const std::string& name, double value) override {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        metrics_[module + "." + name] = value;
    }

    std::map<std::string, double> get_performance_snapshot() const override {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        return metrics_;
    }
};

// ── Nexus Tests ──────────────────────────────────────────────────────────

class NexusTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
    }
};

TEST_F(NexusTest, DefaultEngineIsRegistered) {
    auto nexus = Registry<INexus>::instance().create("default");
    ASSERT_NE(nexus, nullptr);
}

TEST_F(NexusTest, SubmitTaskExecutes) {
    auto nexus = Registry<INexus>::instance().create("default");
    ASSERT_NE(nexus, nullptr);

    std::atomic<bool> executed{false};
    NexusTask task;
    task.id = "test_task";
    task.owner_module = "test";
    task.priority = TaskPriority::Normal;
    task.work = [&executed]() { executed = true; };

    nexus->submit_task(task);

    // Give the worker thread time to pick it up
    for (int i = 0; i < 50 && !executed; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    EXPECT_TRUE(executed);
}

TEST_F(NexusTest, LogicalTickMonotonicallyIncreases) {
    auto nexus = Registry<INexus>::instance().create("default");
    ASSERT_NE(nexus, nullptr);

    uint64_t t1 = nexus->logical_tick();
    uint64_t t2 = nexus->logical_tick();
    uint64_t t3 = nexus->logical_tick();

    EXPECT_GT(t2, t1);
    EXPECT_GT(t3, t2);
}

TEST_F(NexusTest, NowReturnsCurrentTime) {
    auto nexus = Registry<INexus>::instance().create("default");
    auto before = std::chrono::system_clock::now();
    auto n = nexus->now();
    auto after = std::chrono::system_clock::now();

    EXPECT_GE(n, before);
    EXPECT_LE(n, after);
}

TEST_F(NexusTest, ReportAndReadMetrics) {
    auto nexus = Registry<INexus>::instance().create("default");
    nexus->report_metric("scanner", "files_per_sec", 1500.0);
    nexus->report_metric("hasher", "mb_per_sec", 250.0);

    auto snapshot = nexus->get_performance_snapshot();
    EXPECT_EQ(snapshot.count("scanner.files_per_sec"), 1u);
    EXPECT_EQ(snapshot.count("hasher.mb_per_sec"), 1u);
}

TEST_F(NexusTest, AcquireAndReleaseResource) {
    auto nexus = Registry<INexus>::instance().create("default");
    ASSERT_NE(nexus, nullptr);

    bool acquired = nexus->acquire_resource(ResourceType::DiskIO, "test_module");
    EXPECT_TRUE(acquired);

    nexus->release_resource(ResourceType::DiskIO, "test_module");
    // Should be able to acquire again
    bool reacquired = nexus->acquire_resource(ResourceType::DiskIO, "test_module2");
    EXPECT_TRUE(reacquired);
    nexus->release_resource(ResourceType::DiskIO, "test_module2");
}

// ── AutoCleanupManager Tests ─────────────────────────────────────────────

class AutoCleanupTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_cleanup_" + unique_id);
        std::filesystem::create_directories(test_dir);
    }

    void TearDown() override {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    void create_file(const std::filesystem::path& path, const std::string& content = "data") {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream ofs(path);
        ofs << content;
    }

    std::filesystem::path test_dir;
};

TEST_F(AutoCleanupTest, RunSmartPurgeSubmitsTask) {
    auto pruner = std::shared_ptr<IDataPruner>(
        Registry<IDataPruner>::instance().create("default")
    );
    auto copy_engine = std::make_shared<EnhancedCopyEngine>();
    MockNexus nexus;

    AutoCleanupManager mgr(pruner, copy_engine, nexus);

    create_file(test_dir / "temp.tmp", "temp");
    mgr.run_smart_purge(test_dir, 0.5);

    std::lock_guard<std::mutex> lock(nexus.tasks_mutex);
    ASSERT_EQ(nexus.submitted_tasks.size(), 1u);
    EXPECT_EQ(nexus.submitted_tasks[0].id, "auto_cleanup_purge");
}

TEST_F(AutoCleanupTest, ScheduleMaintenanceDoesNotCrash) {
    auto pruner = std::shared_ptr<IDataPruner>(
        Registry<IDataPruner>::instance().create("default")
    );
    auto copy_engine = std::make_shared<EnhancedCopyEngine>();
    MockNexus nexus;

    AutoCleanupManager mgr(pruner, copy_engine, nexus);
    EXPECT_NO_THROW(mgr.schedule_maintenance(7));
}
