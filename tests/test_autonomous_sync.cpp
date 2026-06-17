#include <gtest/gtest.h>
#include "fo/core/autonomous_sync_interface.hpp"
#include "fo/core/swarm_engine_interface.hpp"
#include "fo/core/nexus_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>

using namespace fo::core;

class AutonomousSyncTest : public ::testing::Test {
protected:
    std::filesystem::path local_dir = "test_sync_local";

    void SetUp() override {
        register_all_providers();
        std::filesystem::create_directories(local_dir);
    }

    void TearDown() override {
        Registry<IAutonomousSyncService>::instance().clear_shared_instances();
        Registry<ISwarmEngine>::instance().clear_shared_instances();
        Registry<INexus>::instance().clear_shared_instances();
        std::filesystem::remove_all(local_dir);
    }
};

TEST_F(AutonomousSyncTest, BasicUploadAndVersioning) {
    auto sync = Registry<IAutonomousSyncService>::instance().get_shared("default");
    ASSERT_NE(sync, nullptr);
    sync->set_storage_path(local_dir / "sync.db");

    std::filesystem::path file1 = local_dir / "doc1.txt";
    { std::ofstream ofs(file1); ofs << "v1 content"; }

    EXPECT_TRUE(sync->upload_file(file1, "remote/doc1.txt"));
    
    auto history = sync->get_version_history("remote/doc1.txt");
    ASSERT_EQ(history.size(), 1);
    EXPECT_EQ(history[0].version_id, "v1");

    // Upload same content (should be ignored by logic but service currently returns true and skips tracking if checksum matches)
    EXPECT_TRUE(sync->upload_file(file1, "remote/doc1.txt"));
    history = sync->get_version_history("remote/doc1.txt");
    EXPECT_EQ(history.size(), 1); // Still v1

    // Update content and upload
    { std::ofstream ofs(file1); ofs << "v2 content"; }
    EXPECT_TRUE(sync->upload_file(file1, "remote/doc1.txt"));
    history = sync->get_version_history("remote/doc1.txt");
    EXPECT_EQ(history.size(), 2);
    EXPECT_EQ(history[1].version_id, "v2");
}

TEST_F(AutonomousSyncTest, CollisionResolution) {
    auto sync = Registry<IAutonomousSyncService>::instance().get_shared("default");
    ASSERT_NE(sync, nullptr);
    sync->set_storage_path(local_dir / "sync_collision.db");
    
    std::filesystem::path f1 = local_dir / "f1";
    { std::ofstream ofs(f1); ofs << "initial"; }

    sync->upload_file(f1, "remote/f1"); // v1
    
    // Simulate v2
    { std::ofstream ofs(f1); ofs << "new content"; }
    sync->upload_file(f1, "remote/f1"); // v2

    EXPECT_TRUE(sync->resolve_collision("remote/f1", "v1"));
    auto history = sync->get_version_history("remote/f1");
    EXPECT_EQ(history.size(), 3);
    EXPECT_TRUE(history.back().version_id.find("resolved") != std::string::npos);
}

TEST_F(AutonomousSyncTest, ConcurrentAccess) {
    auto sync = Registry<IAutonomousSyncService>::instance().get_shared("default");
    sync->set_storage_path(local_dir / "sync_concurrent.db");
    const int num_threads = 10;
    const int iterations = 50;

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, i]() {
            for (int j = 0; j < iterations; ++j) {
                std::filesystem::path p = local_dir / ("thread_" + std::to_string(i) + "_" + std::to_string(j));
                { std::ofstream ofs(p); ofs << "data"; }
                sync->upload_file(p, "remote/shared_space");
            }
        });
    }

    for (auto& t : threads) t.join();

    auto history = sync->get_version_history("remote/shared_space");
    EXPECT_GE(history.size(), 1);
}

TEST_F(AutonomousSyncTest, LiveEnvironmentValidation) {
    auto sync = Registry<IAutonomousSyncService>::instance().get_shared("default");
    ASSERT_NE(sync, nullptr);
    sync->set_storage_path(local_dir / "sync_live.db");

    std::filesystem::path test_root = local_dir / "live_env";
    EXPECT_TRUE(sync->validate_live_environment(test_root));
}

TEST_F(AutonomousSyncTest, ContentVerification) {
    auto sync = Registry<IAutonomousSyncService>::instance().get_shared("default");
    sync->set_storage_path(local_dir / "sync_content.db");
    std::filesystem::path f = local_dir / "content.txt";
    std::string rid = "remote/content.txt";

    { std::ofstream ofs(f); ofs << "content 1"; }
    sync->upload_file(f, rid);
    auto history = sync->get_version_history(rid);
    ASSERT_FALSE(history.empty());
    auto h1 = history.back().checksum;

    { std::ofstream ofs(f); ofs << "content 2"; }
    sync->upload_file(f, rid);
    history = sync->get_version_history(rid);
    ASSERT_GE(history.size(), 2);
    auto h2 = history.back().checksum;

    EXPECT_NE(h1, h2);
    EXPECT_EQ(history.size(), 2);
}
