#include <gtest/gtest.h>
#include "fo/core/treemap_engine_interface.hpp"
#include "fo/core/data_pruner_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/provider_registration.hpp"
#include <filesystem>
#include <fstream>

using namespace fo::core;

// ── TreemapEngine Tests ──────────────────────────────────────────────────

class TreemapEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_treemap_" + unique_id);
        std::filesystem::create_directories(test_dir);
    }

    void TearDown() override {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    void create_file(const std::filesystem::path& path, size_t size) {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream ofs(path, std::ios::binary);
        std::vector<char> data(size, 'x');
        ofs.write(data.data(), data.size());
    }

    std::filesystem::path test_dir;
};

TEST_F(TreemapEngineTest, DefaultEngineIsRegistered) {
    auto engine = Registry<ITreemapEngine>::instance().create("default");
    ASSERT_NE(engine, nullptr);
}

TEST_F(TreemapEngineTest, BuildTreeSingleFile) {
    create_file(test_dir / "file.txt", 100);

    auto engine = Registry<ITreemapEngine>::instance().create("default");
    auto tree = engine->build_tree(test_dir);

    EXPECT_EQ(tree.size, 100u);
    EXPECT_TRUE(tree.children.empty());
}

TEST_F(TreemapEngineTest, BuildTreeMultipleFiles) {
    create_file(test_dir / "a.txt", 100);
    create_file(test_dir / "b.txt", 200);
    create_file(test_dir / "c.txt", 300);

    auto engine = Registry<ITreemapEngine>::instance().create("default");
    auto tree = engine->build_tree(test_dir);

    EXPECT_EQ(tree.size, 600u);
}

TEST_F(TreemapEngineTest, BuildTreeWithSubdirectories) {
    create_file(test_dir / "root.txt", 50);
    create_file(test_dir / "sub1" / "file1.txt", 100);
    create_file(test_dir / "sub1" / "file2.txt", 200);
    create_file(test_dir / "sub2" / "file3.txt", 300);

    auto engine = Registry<ITreemapEngine>::instance().create("default");
    auto tree = engine->build_tree(test_dir);

    EXPECT_EQ(tree.size, 650u);
    EXPECT_EQ(tree.children.size(), 2u);

    // Children should be sorted by size descending
    // sub2 has 300, sub1 has 300 — both 300
    EXPECT_GE(tree.children[0].size, tree.children[1].size);
}

TEST_F(TreemapEngineTest, BuildTreeRespectsMaxDepth) {
    create_file(test_dir / "a" / "b" / "c" / "deep.txt", 50);

    auto engine = Registry<ITreemapEngine>::instance().create("default");
    auto tree = engine->build_tree(test_dir, 1); // depth=1

    EXPECT_EQ(tree.size, 50u);
    // At depth 1, should only go one level into "a"
    ASSERT_GE(tree.children.size(), 1u);
    // The child "a" should not have children since we hit max depth
    // (at max_depth=0 it counts recursively instead)
}

TEST_F(TreemapEngineTest, BuildTreeEmptyDirectory) {
    auto engine = Registry<ITreemapEngine>::instance().create("default");
    auto tree = engine->build_tree(test_dir);

    EXPECT_EQ(tree.size, 0u);
    EXPECT_TRUE(tree.children.empty());
}

TEST_F(TreemapEngineTest, BuildTreeNonExistentPath) {
    auto engine = Registry<ITreemapEngine>::instance().create("default");
    auto tree = engine->build_tree("/nonexistent/path/xyz");

    EXPECT_EQ(tree.size, 0u);
}

TEST_F(TreemapEngineTest, CalculateLayoutSetsCoordinates) {
    create_file(test_dir / "big.txt", 1000);
    create_file(test_dir / "small.txt", 500);
    std::filesystem::create_directories(test_dir / "subdir");
    create_file(test_dir / "subdir" / "mid.txt", 750);

    auto engine = Registry<ITreemapEngine>::instance().create("default");
    auto tree = engine->build_tree(test_dir);

    engine->calculate_layout(tree, 0, 0, 800, 600);

    EXPECT_DOUBLE_EQ(tree.x, 0.0);
    EXPECT_DOUBLE_EQ(tree.y, 0.0);
    EXPECT_DOUBLE_EQ(tree.width, 800.0);
    EXPECT_DOUBLE_EQ(tree.height, 600.0);

    // Children should have layout
    for (const auto& child : tree.children) {
        EXPECT_GT(child.width + child.height, 0.0);
    }
}

TEST_F(TreemapEngineTest, LayoutChildrenDontExceedParent) {
    create_file(test_dir / "a.txt", 100);
    create_file(test_dir / "b.txt", 200);
    create_file(test_dir / "c.txt", 150);

    auto engine = Registry<ITreemapEngine>::instance().create("default");
    auto tree = engine->build_tree(test_dir);

    engine->calculate_layout(tree, 0, 0, 100, 100);

    for (const auto& child : tree.children) {
        EXPECT_LE(child.x + child.width, tree.x + tree.width + 1.0);
        EXPECT_LE(child.y + child.height, tree.y + tree.height + 1.0);
    }
}

// ── DataPruner Tests ─────────────────────────────────────────────────────

class DataPrunerTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_pruner_" + unique_id);
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

TEST_F(DataPrunerTest, DefaultPrunerIsRegistered) {
    auto pruner = Registry<IDataPruner>::instance().create("default");
    ASSERT_NE(pruner, nullptr);
}

TEST_F(DataPrunerTest, DetectsTempFiles) {
    create_file(test_dir / "cache.tmp", "temp data");
    create_file(test_dir / "old.bak", "backup");
    create_file(test_dir / "log.log", "log entries");

    auto pruner = Registry<IDataPruner>::instance().create("default");
    pruner->set_min_confidence(0.5);
    auto suggestions = pruner->analyze(test_dir);

    EXPECT_GE(suggestions.size(), 2u); // .tmp and .bak at minimum

    bool found_tmp = false;
    for (const auto& s : suggestions) {
        if (s.path.filename() == "cache.tmp") {
            found_tmp = true;
            EXPECT_EQ(s.reason, "Temporary File");
            EXPECT_DOUBLE_EQ(s.confidence, 0.95);
        }
    }
    EXPECT_TRUE(found_tmp);
}

TEST_F(DataPrunerTest, DetectsIncompleteDownloads) {
    create_file(test_dir / "video.mp4.part", "partial");
    create_file(test_dir / "data.crdownload", "partial");

    auto pruner = Registry<IDataPruner>::instance().create("default");
    auto suggestions = pruner->analyze(test_dir);

    bool found_part = false;
    for (const auto& s : suggestions) {
        if (s.path.filename() == "video.mp4.part") {
            found_part = true;
            EXPECT_EQ(s.reason, "Incomplete Download");
            EXPECT_TRUE(s.safe_to_auto_delete);
        }
    }
    EXPECT_TRUE(found_part);
}

TEST_F(DataPrunerTest, NormalFileBelowThreshold) {
    create_file(test_dir / "important.doc", "work document");
    create_file(test_dir / "photo.jpg", "image data");

    auto pruner = Registry<IDataPruner>::instance().create("default");
    pruner->set_min_confidence(0.5);
    auto suggestions = pruner->analyze(test_dir);

    // Normal files should not be flagged
    for (const auto& s : suggestions) {
        EXPECT_NE(s.path.filename(), "important.doc");
        EXPECT_NE(s.path.filename(), "photo.jpg");
    }
}

TEST_F(DataPrunerTest, MinConfidenceFilter) {
    create_file(test_dir / "test.tmp", "temp");     // confidence 0.95
    create_file(test_dir / "normal.txt", "normal"); // confidence 0

    auto pruner = Registry<IDataPruner>::instance().create("default");

    // High threshold — should only get very high confidence
    pruner->set_min_confidence(0.9);
    auto high = pruner->analyze(test_dir);
    for (const auto& s : high) {
        EXPECT_GE(s.confidence, 0.9);
    }

    // Low threshold — should get more
    pruner->set_min_confidence(0.5);
    auto low = pruner->analyze(test_dir);
    EXPECT_GE(low.size(), high.size());
}

TEST_F(DataPrunerTest, CustomRule) {
    create_file(test_dir / "readme.md", "# Readme");

    auto pruner = Registry<IDataPruner>::instance().create("default");
    pruner->set_min_confidence(0.5);

    // No match initially
    auto before = pruner->analyze(test_dir);
    EXPECT_TRUE(before.empty());

    // Add custom rule
    pruner->add_custom_rule("Readme files", R"(.*\.md$)", 0.8);

    auto after = pruner->analyze(test_dir);
    ASSERT_GE(after.size(), 1u);
    EXPECT_EQ(after[0].reason, "Readme files");
}

TEST_F(DataPrunerTest, EmptyDirectoryReturnsNoSuggestions) {
    auto pruner = Registry<IDataPruner>::instance().create("default");
    auto suggestions = pruner->analyze(test_dir);
    EXPECT_TRUE(suggestions.empty());
}

TEST_F(DataPrunerTest, NonExistentDirectoryReturnsEmpty) {
    auto pruner = Registry<IDataPruner>::instance().create("default");
    auto suggestions = pruner->analyze("/nonexistent/path");
    EXPECT_TRUE(suggestions.empty());
}

TEST_F(DataPrunerTest, SuggestionHasCorrectFields) {
    create_file(test_dir / "waste.tmp", "temp data here");

    auto pruner = Registry<IDataPruner>::instance().create("default");
    auto suggestions = pruner->analyze(test_dir);

    ASSERT_GE(suggestions.size(), 1u);
    const auto& s = suggestions[0];
    EXPECT_FALSE(s.path.empty());
    EXPECT_FALSE(s.reason.empty());
    EXPECT_GT(s.confidence, 0.0);
    EXPECT_LE(s.confidence, 1.0);
    EXPECT_GT(s.size, 0u);
}
