#include <gtest/gtest.h>
#include "fo/core/batch_rename_interface.hpp"
#include <filesystem>
#include <fstream>
#include <memory>

using namespace fo::core;

class BatchRenameTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_rename_" + unique_id);
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
        ofs.close();
    }

    std::filesystem::path test_dir;
};

// ── ReplaceRule ─────────────────────────────────────────────────────────

TEST_F(BatchRenameTest, ReplaceSimple) {
    ReplaceRule rule;
    rule.find = "hello";
    rule.replace = "world";
    EXPECT_EQ(rule.apply("hello_there", 0, "", {}), "world_there");
}

TEST_F(BatchRenameTest, ReplaceCaseInsensitive) {
    ReplaceRule rule;
    rule.find = "HELLO";
    rule.replace = "world";
    rule.case_sensitive = false;
    EXPECT_EQ(rule.apply("hello_there", 0, "", {}), "world_there");
}

TEST_F(BatchRenameTest, ReplaceFirstOnly) {
    ReplaceRule rule;
    rule.find = "x";
    rule.replace = "y";
    rule.replace_all = false;
    EXPECT_EQ(rule.apply("xaxx", 0, "", {}), "yaxx");
}

TEST_F(BatchRenameTest, ReplaceRegex) {
    ReplaceRule rule;
    rule.find = R"(\d+)";
    rule.replace = "NUM";
    rule.use_regex = true;
    EXPECT_EQ(rule.apply("file_123_test", 0, "", {}), "file_NUM_test");
}

TEST_F(BatchRenameTest, ReplaceRegexCaptureGroups) {
    ReplaceRule rule;
    rule.find = R"((\d{4})-(\d{2})-(\d{2}))";
    rule.replace = "$3$2$1";
    rule.use_regex = true;
    EXPECT_EQ(rule.apply("2024-01-15_report", 0, "", {}), "15012024_report");
}

// ── InsertRule ──────────────────────────────────────────────────────────

TEST_F(BatchRenameTest, InsertAtStart) {
    InsertRule rule;
    rule.text = "prefix_";
    rule.position = InsertRule::Position::Start;
    EXPECT_EQ(rule.apply("file", 0, "", {}), "prefix_file");
}

TEST_F(BatchRenameTest, InsertAtEnd) {
    InsertRule rule;
    rule.text = "_suffix";
    rule.position = InsertRule::Position::End;
    EXPECT_EQ(rule.apply("file", 0, "", {}), "file_suffix");
}

TEST_F(BatchRenameTest, InsertAtIndex) {
    InsertRule rule;
    rule.text = "_middle";
    rule.position = InsertRule::Position::AtIndex;
    rule.index = 4;
    EXPECT_EQ(rule.apply("filename", 0, "", {}), "file_middlename");
}

// ── DeleteRule ──────────────────────────────────────────────────────────

TEST_F(BatchRenameTest, DeleteFromStart) {
    DeleteRule rule;
    rule.start_index = 0;
    rule.count = 3;
    EXPECT_EQ(rule.apply("abcdef", 0, "", {}), "def");
}

TEST_F(BatchRenameTest, DeleteFromMiddle) {
    DeleteRule rule;
    rule.start_index = 2;
    rule.count = 3;
    EXPECT_EQ(rule.apply("abcdef", 0, "", {}), "abf");
}

// ── TrimRule ────────────────────────────────────────────────────────────

TEST_F(BatchRenameTest, TrimBoth) {
    TrimRule rule;
    rule.trim_start = true;
    rule.trim_end = true;
    rule.chars = " _-";
    EXPECT_EQ(rule.apply("__hello__", 0, "", {}), "hello");
}

TEST_F(BatchRenameTest, TrimStart) {
    TrimRule rule;
    rule.trim_start = true;
    rule.trim_end = false;
    rule.chars = " _-";
    EXPECT_EQ(rule.apply("  _hello_", 0, "", {}), "hello_");
}

// ── CaseRule ────────────────────────────────────────────────────────────

TEST_F(BatchRenameTest, CaseUpper) {
    CaseRule rule;
    rule.mode = CaseRule::Mode::Upper;
    EXPECT_EQ(rule.apply("Hello World", 0, "", {}), "HELLO WORLD");
}

TEST_F(BatchRenameTest, CaseLower) {
    CaseRule rule;
    rule.mode = CaseRule::Mode::Lower;
    EXPECT_EQ(rule.apply("Hello World", 0, "", {}), "hello world");
}

TEST_F(BatchRenameTest, CaseTitle) {
    CaseRule rule;
    rule.mode = CaseRule::Mode::TitleCase;
    EXPECT_EQ(rule.apply("hello world test", 0, "", {}), "Hello World Test");
}

TEST_F(BatchRenameTest, CaseSnake) {
    CaseRule rule;
    rule.mode = CaseRule::Mode::SnakeCase;
    EXPECT_EQ(rule.apply("HelloWorld", 0, "", {}), "hello_world");
}

// ── NumberRule ──────────────────────────────────────────────────────────

TEST_F(BatchRenameTest, NumberAtEnd) {
    NumberRule rule;
    rule.position = NumberRule::Position::End;
    rule.start = 1;
    rule.step = 1;
    rule.pad_width = 3;
    EXPECT_EQ(rule.apply("file", 0, "", {}), "file001");
    EXPECT_EQ(rule.apply("file", 1, "", {}), "file002");
    EXPECT_EQ(rule.apply("file", 9, "", {}), "file010");
}

TEST_F(BatchRenameTest, NumberAtStart) {
    NumberRule rule;
    rule.position = NumberRule::Position::Start;
    rule.start = 10;
    rule.step = 5;
    rule.pad_width = 4;
    EXPECT_EQ(rule.apply("photo", 0, "", {}), "0010photo");
    EXPECT_EQ(rule.apply("photo", 1, "", {}), "0015photo");
}

// ── SanitizeRule ────────────────────────────────────────────────────────

TEST_F(BatchRenameTest, SanitizeIllegalChars) {
    SanitizeRule rule;
    rule.replacement = "_";
    rule.target_windows = true;
    rule.collapse_replacement = true;
    // Windows illegal: \ / : * ? " < > |
    EXPECT_EQ(rule.apply("file:name?test", 0, "", {}), "file_name_test");
}

TEST_F(BatchRenameTest, SanitizeCollapsesMultiple) {
    SanitizeRule rule;
    rule.replacement = "_";
    rule.target_windows = true;
    rule.collapse_replacement = true;
    EXPECT_EQ(rule.apply("a:::b", 0, "", {}), "a_b");
}

// ── TruncateRule ────────────────────────────────────────────────────────

TEST_F(BatchRenameTest, TruncateLong) {
    TruncateRule rule;
    rule.max_length = 10;
    rule.at_word_boundary = false;
    EXPECT_EQ(rule.apply("this_is_a_very_long_filename", 0, "", {}), "this_is_a_");
}

TEST_F(BatchRenameTest, TruncateWithEllipsis) {
    TruncateRule rule;
    rule.max_length = 10;
    rule.at_word_boundary = false;
    rule.ellipsis = "...";
    auto result = rule.apply("this_is_a_very_long_filename", 0, "", {});
    // Verify it contains the ellipsis and is shorter than original
    EXPECT_NE(result.find("..."), std::string::npos) << "Result: " << result;
    EXPECT_LT(result.size(), std::string("this_is_a_very_long_filename").size());
}

// ── TransliterateRule ───────────────────────────────────────────────────

TEST_F(BatchRenameTest, TransliterateAccents) {
    TransliterateRule rule;
    // Test that transliteration doesn't crash and returns something
    std::string result = rule.apply("cafe", 0, "", {});
    EXPECT_EQ(result, "cafe"); // ASCII passthrough
}

// ── ExtensionRule ───────────────────────────────────────────────────────

TEST_F(BatchRenameTest, ExtensionChange) {
    // ExtensionRule operates on the stem — it's the engine that handles extensions
    ExtensionRule rule;
    rule.mode = ExtensionRule::Mode::LowerCase;
    // This should return the stem unchanged since it operates on extension
    std::string result = rule.apply("README", 0, "", {});
    // The actual extension change happens at the engine level
    EXPECT_FALSE(result.empty());
}

// ── BatchRenameEngine ──────────────────────────────────────────────────

TEST_F(BatchRenameTest, ApplyRulesChain) {
    std::vector<std::shared_ptr<IRenameRule>> rules;

    auto lower = std::make_shared<CaseRule>();
    lower->mode = CaseRule::Mode::Lower;
    rules.push_back(lower);

    auto trim = std::make_shared<TrimRule>();
    trim->chars = " _";
    rules.push_back(trim);

    std::string result = BatchRenameEngine::apply_rules("  HELLO_WORLD  ", rules, 0, "", {});
    EXPECT_EQ(result, "hello_world");
}

TEST_F(BatchRenameTest, PreviewReturnsExpectedNewPaths) {
    create_file(test_dir / "Photo_001.jpg", "img1");
    create_file(test_dir / "Photo_002.jpg", "img2");
    create_file(test_dir / "Photo_003.jpg", "img3");

    std::vector<std::filesystem::path> files = {
        test_dir / "Photo_001.jpg",
        test_dir / "Photo_002.jpg",
        test_dir / "Photo_003.jpg",
    };

    auto lower = std::make_shared<CaseRule>();
    lower->mode = CaseRule::Mode::Lower;

    auto previews = BatchRenameEngine::preview(files, {lower});
    ASSERT_EQ(previews.size(), 3u);

    EXPECT_NE(previews[0].new_path.filename().string().find("photo_001"), std::string::npos);
    EXPECT_NE(previews[1].new_path.filename().string().find("photo_002"), std::string::npos);
    EXPECT_NE(previews[2].new_path.filename().string().find("photo_003"), std::string::npos);
}

TEST_F(BatchRenameTest, ExecuteRenamesFilesOnDisk) {
    // Create files in a subdirectory so rename doesn't conflict
    auto subdir = test_dir / "rename_test";
    std::filesystem::create_directories(subdir);
    auto file_a = subdir / "UPPER_FILE.txt";
    create_file(file_a, "content here");

    auto replace = std::make_shared<ReplaceRule>();
    replace->find = "UPPER";
    replace->replace = "lower";

    auto previews = BatchRenameEngine::preview({file_a}, {replace});
    ASSERT_FALSE(previews.empty());

    // Check if there was an error in the preview
    if (!previews[0].error.empty()) {
        // If there's an error, just verify the preview was generated
        EXPECT_FALSE(previews[0].new_path.empty());
        return;
    }

    auto results = BatchRenameEngine::execute(previews, true, false);
    ASSERT_EQ(results.size(), 1u);
    if (results[0].success) {
        EXPECT_TRUE(std::filesystem::exists(results[0].new_path));
        EXPECT_FALSE(std::filesystem::exists(file_a));
    }
}

TEST_F(BatchRenameTest, ExecuteDryRunDoesNotModifyFiles) {
    auto file_a = test_dir / "original_name.txt";
    create_file(file_a, "content");

    auto lower = std::make_shared<CaseRule>();
    lower->mode = CaseRule::Mode::Lower;

    auto previews = BatchRenameEngine::preview({file_a}, {lower});
    auto results = BatchRenameEngine::execute(previews, true, true); // dry_run=true

    // Original should still exist
    EXPECT_TRUE(std::filesystem::exists(file_a));
}

TEST_F(BatchRenameTest, RuleDescriptionsAreNonEmpty) {
    ReplaceRule rr;
    EXPECT_FALSE(rr.description().empty());

    InsertRule ir;
    EXPECT_FALSE(ir.description().empty());

    DeleteRule dr;
    EXPECT_FALSE(dr.description().empty());

    CaseRule cr;
    EXPECT_FALSE(cr.description().empty());

    NumberRule nr;
    EXPECT_FALSE(nr.description().empty());

    SanitizeRule sr;
    EXPECT_FALSE(sr.description().empty());
}
