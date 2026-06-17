#include <gtest/gtest.h>
#include "fo/core/engine.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include "fo/core/swarm_engine_interface.hpp"
#include <filesystem>
#include <fstream>
#include <thread>

namespace fs = std::filesystem;

class DistributedSyncTest : public ::testing::Test {
protected:
    void SetUp() override {
        fo::core::register_all_providers();
        test_root_ = fs::current_path() / "test_dist_sync";
        node1_root_ = test_root_ / "node1";
        node2_root_ = test_root_ / "node2";
        fs::create_directories(node1_root_);
        fs::create_directories(node2_root_);
    }

    void TearDown() override {
        fo::core::Registry<fo::core::IAutonomousSyncService>::instance().clear_shared_instances();
        fo::core::Registry<fo::core::IAutonomousDevProtocol>::instance().clear_shared_instances();
        fo::core::Registry<fo::core::ISwarmEngine>::instance().clear_shared_instances();
        fo::core::Registry<fo::core::INexus>::instance().clear_shared_instances();
        fs::remove_all(test_root_);
    }

    fs::path test_root_;
    fs::path node1_root_;
    fs::path node2_root_;
};

TEST_F(DistributedSyncTest, RealTimeSyncSimulation) {
    fo::core::EngineConfig cfg1;
    cfg1.db_path = (test_root_ / "node1.db").string();
    fo::core::Engine engine1(cfg1);

    fo::core::EngineConfig cfg2;
    cfg2.db_path = (test_root_ / "node2.db").string();
    fo::core::Engine engine2(cfg2);

    // 1. Start sync daemon on Node 1
    engine1.sync_service().start_sync_daemon(node1_root_);

    // Give time for watcher to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 2. Modify a file on Node 1
    fs::path test_file = node1_root_ / "shared.txt";
    {
        std::ofstream ofs(test_file);
        ofs << "Initial content";
        ofs.flush();
    }

    // Give time for watcher and sync
    std::string remote_id = "sync/shared.txt";
    std::vector<fo::core::FileVersion> history1;
    for (int i = 0; i < 40; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        history1 = engine1.sync_service().get_version_history(remote_id);
        if (history1.size() >= 1) break;
    }

    // 3. Verify Node 1 tracked the version
    EXPECT_GE(history1.size(), 1);

    // 4. Automatic peer update: Node 2 should have received the broadcast from Node 1
    // and automatically triggered a download (simulated version entry).

    std::vector<fo::core::FileVersion> history2;
    for (int i = 0; i < 40; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        history2 = engine2.sync_service().get_version_history(remote_id);
        if (history2.size() >= 1) {
            bool found_downloaded = false;
            for (auto const& v : history2) {
                if (v.version_id.find("downloaded") != std::string::npos) {
                    found_downloaded = true;
                    break;
                }
            }
            if (found_downloaded) break;
        }
    }

    EXPECT_GE(history2.size(), 1);
    bool found_downloaded = false;
    for (auto const& v : history2) {
        if (v.version_id.find("downloaded") != std::string::npos) {
            found_downloaded = true;
            break;
        }
    }
    EXPECT_TRUE(found_downloaded);
}
