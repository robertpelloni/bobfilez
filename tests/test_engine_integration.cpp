/// @file test_engine_integration.cpp
/// @brief Integration tests for the Engine class — full scan + duplicates pipeline.

#include <gtest/gtest.h>
#include "fo/core/engine.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>

using namespace fo::core;

class EngineIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_engine_" + unique_id);
        src_dir = test_dir / "source";
        std::filesystem::create_directories(src_dir);

        db_path = test_dir / "test.db";
    }

    void TearDown() override {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    void create_file(const std::filesystem::path& path, const std::string& content) {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream ofs(path, std::ios::binary);
        ofs << content;
    }

    std::filesystem::path test_dir;
    std::filesystem::path src_dir;
    std::filesystem::path db_path;
};

TEST_F(EngineIntegrationTest, ScanEmptyDirectory) {
    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({src_dir}, {}, false);
    EXPECT_TRUE(files.empty());
}

TEST_F(EngineIntegrationTest, ScanSingleFile) {
    create_file(src_dir / "hello.txt", "Hello World");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({src_dir}, {}, false);
    ASSERT_EQ(files.size(), 1u);
    EXPECT_NE(files[0].uri.find("hello.txt"), std::string::npos);
    EXPECT_EQ(files[0].size, 11u); // "Hello World"
}

TEST_F(EngineIntegrationTest, ScanMultipleFiles) {
    create_file(src_dir / "a.txt", "aaa");
    create_file(src_dir / "b.txt", "bbbb");
    create_file(src_dir / "c.txt", "ccccc");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({src_dir}, {}, false);
    EXPECT_EQ(files.size(), 3u);
}

TEST_F(EngineIntegrationTest, ScanWithExtensionFilter) {
    create_file(src_dir / "data.txt", "text");
    create_file(src_dir / "image.jpg", "jpg");
    create_file(src_dir / "doc.pdf", "pdf");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({src_dir}, {".txt"}, false);
    EXPECT_EQ(files.size(), 1u);
}

TEST_F(EngineIntegrationTest, ScanNestedDirectories) {
    create_file(src_dir / "root.txt", "root");
    create_file(src_dir / "sub" / "nested.txt", "nested");
    create_file(src_dir / "sub" / "deep" / "deep.txt", "deep");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({src_dir}, {}, false);
    EXPECT_EQ(files.size(), 3u);
}

TEST_F(EngineIntegrationTest, ScanRespectsIgnoreRules) {
    create_file(src_dir / "keep.txt", "keep");
    create_file(src_dir / "temp.tmp", "temp");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    engine.ignore_repository().add(R"(.*\.tmp)", "temp files");

    auto files = engine.scan({src_dir}, {}, false);
    EXPECT_EQ(files.size(), 1u);
    EXPECT_NE(files[0].uri.find("keep.txt"), std::string::npos);
}

TEST_F(EngineIntegrationTest, FindDuplicatesWithIdenticalFiles) {
    // Create two identical files
    std::string content = "duplicate content here";
    create_file(src_dir / "original.txt", content);
    create_file(src_dir / "copy.txt", content);

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({src_dir}, {}, false);
    EXPECT_EQ(files.size(), 2u);

    auto groups = engine.find_duplicates(files);
    ASSERT_EQ(groups.size(), 1u);
    EXPECT_EQ(groups[0].files.size(), 2u);
    EXPECT_EQ(groups[0].size, content.size());
}

TEST_F(EngineIntegrationTest, FindNoDuplicatesForUniqueFiles) {
    create_file(src_dir / "unique1.txt", "content A");
    create_file(src_dir / "unique2.txt", "content B which is different");
    create_file(src_dir / "unique3.txt", "totally different content here");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({src_dir}, {}, false);
    auto groups = engine.find_duplicates(files);
    EXPECT_TRUE(groups.empty());
}

TEST_F(EngineIntegrationTest, FindMultipleDuplicateGroups) {
    // Group 1: 2 files with same content
    create_file(src_dir / "a1.txt", "group one");
    create_file(src_dir / "a2.txt", "group one");

    // Group 2: 3 files with same content
    create_file(src_dir / "b1.txt", "group two data");
    create_file(src_dir / "b2.txt", "group two data");
    create_file(src_dir / "b3.txt", "group two data");

    // Unique file
    create_file(src_dir / "c.txt", "unique content here");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({src_dir}, {}, false);
    EXPECT_EQ(files.size(), 6u);

    auto groups = engine.find_duplicates(files);
    EXPECT_EQ(groups.size(), 2u);
}

TEST_F(EngineIntegrationTest, ScanCreatesSession) {
    create_file(src_dir / "session.txt", "data");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    // Scan should start and end a session without crashing
    EXPECT_NO_THROW(engine.scan({src_dir}, {}, false));
}

TEST_F(EngineIntegrationTest, ScanUpdatesFileRepository) {
    create_file(src_dir / "repo.txt", "content");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    engine.scan({src_dir}, {}, false);

    // Verify file was stored by looking it up
    auto found = engine.file_repository().get_by_path(src_dir / "repo.txt");
    EXPECT_TRUE(found.has_value());
}

TEST_F(EngineIntegrationTest, RescanDetectsNewFiles) {
    create_file(src_dir / "existing.txt", "old");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files1 = engine.scan({src_dir}, {}, false);
    EXPECT_EQ(files1.size(), 1u);

    // Add a new file
    create_file(src_dir / "new.txt", "new file");

    auto files2 = engine.scan({src_dir}, {}, false);
    EXPECT_EQ(files2.size(), 2u);
}

TEST_F(EngineIntegrationTest, SearchEngineAccessible) {
    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto& se = engine.search_engine();
    (void)se; // Should not crash
}

TEST_F(EngineIntegrationTest, DatabaseIsOpen) {
    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    // Database should be usable
    int count = engine.database().query_int("SELECT COUNT(*) FROM files");
    EXPECT_EQ(count, 0);
}

TEST_F(EngineIntegrationTest, UseAdsCacheConfig) {
    EngineConfig cfg;
    cfg.db_path = db_path.string();
    cfg.use_ads_cache = true;
    Engine engine(cfg);
    EXPECT_TRUE(engine.use_ads_cache());
}

TEST_F(EngineIntegrationTest, UseAdsCacheDefaultOff) {
    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);
    EXPECT_FALSE(engine.use_ads_cache());
}

TEST_F(EngineIntegrationTest, ScanSubdirectoryTree) {
    create_file(src_dir / "a.txt", "a");
    create_file(src_dir / "sub1" / "b.txt", "b");
    create_file(src_dir / "sub1" / "sub2" / "c.txt", "c");
    create_file(src_dir / "sub1" / "sub2" / "sub3" / "d.txt", "d");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({src_dir}, {}, false);
    EXPECT_EQ(files.size(), 4u);
}

TEST_F(EngineIntegrationTest, ScanMultipleRoots) {
    auto root1 = test_dir / "root1";
    auto root2 = test_dir / "root2";
    std::filesystem::create_directories(root1);
    std::filesystem::create_directories(root2);

    create_file(root1 / "file1.txt", "one");
    create_file(root2 / "file2.txt", "two");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({root1, root2}, {}, false);
    EXPECT_EQ(files.size(), 2u);
}

TEST_F(EngineIntegrationTest, DuplicateGroupHasSize) {
    create_file(src_dir / "dup1.txt", "same content exactly");
    create_file(src_dir / "dup2.txt", "same content exactly");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({src_dir}, {}, false);
    auto groups = engine.find_duplicates(files);
    ASSERT_FALSE(groups.empty());
    EXPECT_GT(groups[0].size, 0u);
}
