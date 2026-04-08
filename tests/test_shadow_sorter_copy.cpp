#include <gtest/gtest.h>
#include "fo/core/shadow_sorter_service.hpp"
#include "fo/core/enhanced_fileops_interface.hpp"
#include "fo/core/file_watcher_interface.hpp"
#include "fo/core/rule_engine.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>
#include <memory>
#include <thread>
#include <chrono>

using namespace fo::core;

// ── EnhancedCopyEngine Tests ─────────────────────────────────────────────
//
// NOTE: EnhancedCopyEngine spawns detached threads for each job.
// Tests must wait for file I/O to complete before asserting results.
// The engine is allocated on the heap to avoid use-after-free from
// detached threads outliving the stack frame.

class EnhancedCopyTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_copy_" + unique_id);
        src_dir = test_dir / "src";
        dst_dir = test_dir / "dst";
        std::filesystem::create_directories(src_dir);
        std::filesystem::create_directories(dst_dir);
    }

    void TearDown() override {
        // Give detached threads time to finish before cleanup.
        // EnhancedCopyEngine spawns std::thread(...).detach() in enqueue().
        // The thread may still be running when the test ends, so we need
        // enough time for it to complete before temp files are removed.
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    void create_file(const std::filesystem::path& path, const std::string& content = "data") {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream ofs(path, std::ios::binary);
        ofs << content;
    }

    std::filesystem::path test_dir;
    std::filesystem::path src_dir;
    std::filesystem::path dst_dir;
};

TEST_F(EnhancedCopyTest, EnqueueCopyJob) {
    create_file(src_dir / "test.txt", "hello world");

    auto engine = std::make_unique<EnhancedCopyEngine>();
    EnhancedCopyOptions opts;
    opts.move_mode = false;

    std::string job_id = engine->enqueue({src_dir / "test.txt"}, dst_dir, opts);
    EXPECT_FALSE(job_id.empty());

    // Wait for async copy to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    EXPECT_TRUE(std::filesystem::exists(dst_dir / "test.txt"));
}

TEST_F(EnhancedCopyTest, EnqueueMoveJob) {
    create_file(src_dir / "move_me.txt", "move this");

    auto engine = std::make_unique<EnhancedCopyEngine>();
    EnhancedCopyOptions opts;
    opts.move_mode = true;

    std::string job_id = engine->enqueue({src_dir / "move_me.txt"}, dst_dir, opts);
    EXPECT_FALSE(job_id.empty());

    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    EXPECT_TRUE(std::filesystem::exists(dst_dir / "move_me.txt"));
    EXPECT_FALSE(std::filesystem::exists(src_dir / "move_me.txt"));
}

TEST_F(EnhancedCopyTest, EnqueueMultipleFiles) {
    create_file(src_dir / "a.txt", "aaa");
    create_file(src_dir / "b.txt", "bbb");
    create_file(src_dir / "c.txt", "ccc");

    auto engine = std::make_unique<EnhancedCopyEngine>();
    EnhancedCopyOptions opts;

    std::string job_id = engine->enqueue(
        {src_dir / "a.txt", src_dir / "b.txt", src_dir / "c.txt"},
        dst_dir, opts
    );
    EXPECT_FALSE(job_id.empty());

    std::this_thread::sleep_for(std::chrono::milliseconds(1200));

    EXPECT_TRUE(std::filesystem::exists(dst_dir / "a.txt"));
    EXPECT_TRUE(std::filesystem::exists(dst_dir / "b.txt"));
    EXPECT_TRUE(std::filesystem::exists(dst_dir / "c.txt"));
}

TEST_F(EnhancedCopyTest, CopyPreservesContent) {
    std::string content = "This is the exact content that should be preserved during copy.";
    create_file(src_dir / "exact.txt", content);

    auto engine = std::make_unique<EnhancedCopyEngine>();
    EnhancedCopyOptions opts;
    engine->enqueue({src_dir / "exact.txt"}, dst_dir, opts);

    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    std::ifstream ifs(dst_dir / "exact.txt");
    std::string result((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    EXPECT_EQ(result, content);
}

TEST_F(EnhancedCopyTest, CancelAllDoesNotCrash) {
    create_file(src_dir / "cancel.txt", "data");

    auto engine = std::make_unique<EnhancedCopyEngine>();
    EnhancedCopyOptions opts;

    // Enqueue then immediately cancel — engine must handle this safely
    std::string job_id = engine->enqueue({src_dir / "cancel.txt"}, dst_dir, opts);
    EXPECT_NO_THROW(engine->cancel_all());
    (void)job_id;

    // Wait for detached worker thread to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

TEST_F(EnhancedCopyTest, ClearFinishedDoesNotCrash) {
    create_file(src_dir / "clear.txt", "data");

    auto engine = std::make_unique<EnhancedCopyEngine>();
    EnhancedCopyOptions opts;

    std::string job_id = engine->enqueue({src_dir / "clear.txt"}, dst_dir, opts);
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    EXPECT_NO_THROW(engine->clear_finished());
    (void)job_id;
}

TEST_F(EnhancedCopyTest, GetJobDoesNotCrash) {
    create_file(src_dir / "job_test.txt", "data");

    auto engine = std::make_unique<EnhancedCopyEngine>();
    EnhancedCopyOptions opts;

    std::string job_id = engine->enqueue({src_dir / "job_test.txt"}, dst_dir, opts);
    std::this_thread::sleep_for(std::chrono::milliseconds(800));

    // get_job may return nullptr if the job already finished
    EXPECT_NO_THROW({
        const TransferJob* job = engine->get_job(job_id);
        (void)job;
    });
}

TEST_F(EnhancedCopyTest, AllJobsDoesNotCrash) {
    create_file(src_dir / "list_test.txt", "data");

    auto engine = std::make_unique<EnhancedCopyEngine>();
    EnhancedCopyOptions opts;

    engine->enqueue({src_dir / "list_test.txt"}, dst_dir, opts);
    EXPECT_NO_THROW(engine->all_jobs());
}

// ── ShadowSorterService Tests ────────────────────────────────────────────

class ShadowSorterTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_shadow_" + unique_id);
        inbox_dir = test_dir / "inbox";
        organized_dir = test_dir / "organized";
        std::filesystem::create_directories(inbox_dir);
        std::filesystem::create_directories(organized_dir);
    }

    void TearDown() override {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    std::filesystem::path test_dir;
    std::filesystem::path inbox_dir;
    std::filesystem::path organized_dir;
};

TEST_F(ShadowSorterTest, ConstructsWithDependencies) {
    auto watcher = std::shared_ptr<IFileWatcher>(
        Registry<IFileWatcher>::instance().create("native")
    );
    auto rules = std::make_shared<RuleEngine>();
    auto copy_engine = std::make_shared<EnhancedCopyEngine>();

    ASSERT_NE(watcher, nullptr);
    ASSERT_NE(rules, nullptr);
    ASSERT_NE(copy_engine, nullptr);

    ShadowSorterService service(watcher, rules, copy_engine);
}

TEST_F(ShadowSorterTest, StartAndStopLifecycle) {
    auto watcher = std::shared_ptr<IFileWatcher>(
        Registry<IFileWatcher>::instance().create("native")
    );
    auto rules = std::make_shared<RuleEngine>();
    auto copy_engine = std::make_shared<EnhancedCopyEngine>();

    ShadowSorterService service(watcher, rules, copy_engine);

    service.start({inbox_dir});
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    service.stop();
}

TEST_F(ShadowSorterTest, ServiceWithRulesDoesNotCrash) {
    auto watcher = std::shared_ptr<IFileWatcher>(
        Registry<IFileWatcher>::instance().create("native")
    );
    auto rules = std::make_shared<RuleEngine>();

    OrganizationRule r;
    r.name = "images";
    r.filter_tag = "image";
    r.destination_template = (organized_dir / "images").string() + "/";
    rules->add_rule(r);

    auto copy_engine = std::make_shared<EnhancedCopyEngine>();
    ShadowSorterService service(watcher, rules, copy_engine);

    service.start({inbox_dir});
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    service.stop();
}
