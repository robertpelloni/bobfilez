#include <gtest/gtest.h>
#include "fo/core/scan_session_repository.hpp"
#include "fo/core/self_healing_interface.hpp"
#include "fo/core/hierarchy_engine_interface.hpp"
#include "fo/core/database.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>
#include <memory>

using namespace fo::core;

// ── ScanSessionRepository Tests ──────────────────────────────────────────

class ScanSessionTest : public ::testing::Test {
protected:
    void SetUp() override {
        db.open(":memory:");
        db.migrate();
    }

    DatabaseManager db;
};

TEST_F(ScanSessionTest, StartSessionReturnsPositiveId) {
    ScanSessionRepository repo(db);
    int64_t id = repo.start_session();
    EXPECT_GT(id, 0);
}

TEST_F(ScanSessionTest, EndSessionUpdatesRecord) {
    ScanSessionRepository repo(db);
    int64_t id = repo.start_session();
    EXPECT_NO_THROW(repo.end_session(id, "completed", 42));

    // Verify the record was updated
    int count = db.query_int(
        "SELECT COUNT(*) FROM scan_sessions WHERE id = " + std::to_string(id) + " AND status = 'completed'"
    );
    EXPECT_EQ(count, 1);
}

TEST_F(ScanSessionTest, EndSessionWithScannedCount) {
    ScanSessionRepository repo(db);
    int64_t id = repo.start_session();
    repo.end_session(id, "completed", 100);

    int scanned = db.query_int(
        "SELECT scanned_count FROM scan_sessions WHERE id = " + std::to_string(id)
    );
    EXPECT_EQ(scanned, 100);
}

TEST_F(ScanSessionTest, MultipleSessions) {
    ScanSessionRepository repo(db);
    int64_t id1 = repo.start_session();
    int64_t id2 = repo.start_session();
    int64_t id3 = repo.start_session();

    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);

    repo.end_session(id1, "completed", 10);
    repo.end_session(id2, "failed", 5);
    repo.end_session(id3, "completed", 20);

    int completed = db.query_int("SELECT COUNT(*) FROM scan_sessions WHERE status = 'completed'");
    EXPECT_EQ(completed, 2);
}

TEST_F(ScanSessionTest, SessionHasDuration) {
    ScanSessionRepository repo(db);
    int64_t id = repo.start_session();
    repo.end_session(id, "completed", 5);

    int64_t duration = db.query_int(
        "SELECT duration_ms FROM scan_sessions WHERE id = " + std::to_string(id)
    );
    // Duration should be >= 0 (may be 0 if start and end happen in the same second)
    EXPECT_GE(duration, 0);
}

// ── SelfHealingEngine Tests ─────────────────────────────────────────────

class SelfHealingTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
    }
};

TEST_F(SelfHealingTest, DefaultEngineIsRegistered) {
    auto engine = Registry<ISelfHealingEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

TEST_F(SelfHealingTest, StartAndStopScrub) {
    auto engine = Registry<ISelfHealingEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    // Should not crash
    engine->start_scrub("C:/tmp");
    engine->stop_scrub();
}

TEST_F(SelfHealingTest, HealFileReturnsTrue) {
    auto engine = Registry<ISelfHealingEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    bool result = engine->heal_file("/some/corrupt/file.txt");
    EXPECT_TRUE(result);
}

TEST_F(SelfHealingTest, GetReportsReturnsEmptyInitially) {
    auto engine = Registry<ISelfHealingEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    auto reports = engine->get_reports();
    EXPECT_TRUE(reports.empty());
}

TEST_F(SelfHealingTest, SetScheduleDaysDoesNotCrash) {
    auto engine = Registry<ISelfHealingEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);

    EXPECT_NO_THROW(engine->set_schedule_days(7));
}

// ── HierarchyEngine Tests ───────────────────────────────────────────────

class HierarchyEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_hierarchy_" + unique_id);
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

TEST_F(HierarchyEngineTest, DefaultEngineIsRegistered) {
    auto engine = Registry<IHierarchyEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

TEST_F(HierarchyEngineTest, ProposeHierarchyReturnsNode) {
    create_file(test_dir / "photo.jpg", "img");
    create_file(test_dir / "doc.pdf", "pdf");

    auto engine = Registry<IHierarchyEngine>::instance().create("default");
    auto root = engine->propose_hierarchy({test_dir / "photo.jpg", test_dir / "doc.pdf"});

    EXPECT_EQ(root.folder_name, "Proposed Organization");
    EXPECT_FALSE(root.subfolders.empty());
}

TEST_F(HierarchyEngineTest, ProposeHierarchyGroupsByYear) {
    create_file(test_dir / "file.txt", "data");

    auto engine = Registry<IHierarchyEngine>::instance().create("default");
    auto root = engine->propose_hierarchy({test_dir / "file.txt"});

    // Should have at least one year-based subfolder
    ASSERT_FALSE(root.subfolders.empty());
    // Year should be a valid year (2020+)
    int year = std::stoi(root.subfolders[0].folder_name);
    EXPECT_GE(year, 2020);
    EXPECT_LE(year, 2030);
}

TEST_F(HierarchyEngineTest, ProposeHierarchyGroupsByExtension) {
    create_file(test_dir / "photo.jpg", "img");
    create_file(test_dir / "notes.txt", "text");

    auto engine = Registry<IHierarchyEngine>::instance().create("default");
    auto root = engine->propose_hierarchy({test_dir / "photo.jpg", test_dir / "notes.txt"});

    // Should have year subfolders, each with extension subfolders
    ASSERT_FALSE(root.subfolders.empty());
    auto& year_node = root.subfolders[0];
    EXPECT_FALSE(year_node.subfolders.empty());

    // Collect all extension names
    std::set<std::string> exts;
    for (const auto& type_node : year_node.subfolders) {
        exts.insert(type_node.folder_name);
    }
    EXPECT_TRUE(exts.count("jpg") > 0 || exts.count("txt") > 0);
}

TEST_F(HierarchyEngineTest, ProposeHierarchyEmptyInput) {
    auto engine = Registry<IHierarchyEngine>::instance().create("default");
    auto root = engine->propose_hierarchy({});

    EXPECT_EQ(root.folder_name, "Proposed Organization");
    EXPECT_TRUE(root.subfolders.empty());
}

TEST_F(HierarchyEngineTest, ApplyHierarchyDoesNotCrash) {
    auto engine = Registry<IHierarchyEngine>::instance().create("default");
    HierarchyNode root;
    root.folder_name = "test";
    bool result = engine->apply_hierarchy(root, test_dir / "output");
    EXPECT_TRUE(result);
}

TEST_F(HierarchyEngineTest, SetGranularityDoesNotCrash) {
    auto engine = Registry<IHierarchyEngine>::instance().create("default");
    EXPECT_NO_THROW(engine->set_granularity(0.8));
}
