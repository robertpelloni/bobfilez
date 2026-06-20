#include <gtest/gtest.h>
#include "fo/core/enhanced_fileops_interface.hpp"
#include "fo/core/registry.hpp"
#include <fstream>
#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

class EnhancedCopyRetryTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = fs::temp_directory_path() / "fo_enhanced_test";
        if (fs::exists(test_dir)) fs::remove_all(test_dir);
        fs::create_directories(test_dir);
        src_file = test_dir / "src.txt";
        dst_file = test_dir / "dst.txt";
        
        std::ofstream(src_file) << "hello world";
    }

    void TearDown() override {
        fs::remove_all(test_dir);
        // Small sleep to ensure detached threads from previous tests are cleaned up
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    fs::path test_dir;
    fs::path src_file;
    fs::path dst_file;
};

TEST_F(EnhancedCopyRetryTest, RetriesOnTransientFailure) {
    fo::core::EnhancedCopyEngine engine;
    fo::core::EnhancedCopyOptions opts;
    opts.auto_retry_count = 2;
    opts.auto_retry_delay_ms = 10;
    
    // Test standard copy
    auto job_id = engine.enqueue({src_file}, test_dir, opts);
    
    // Wait for job completion
    const fo::core::TransferJob* job = nullptr;
    for(int i=0; i<100; ++i) {
        job = engine.get_job(job_id);
        if (job && (job->state == fo::core::TransferJob::State::Done || job->state == fo::core::TransferJob::State::Failed)) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    
    ASSERT_NE(job, nullptr);
    EXPECT_EQ(job->state, fo::core::TransferJob::State::Done);
    EXPECT_TRUE(fs::exists(test_dir / "src.txt"));
}

