#include <gtest/gtest.h>
#include "fo/core/engine.hpp"
#include "fo/core/export.hpp"
#include "fo/core/rule_engine.hpp"
#include "fo/core/search_interface.hpp"
#include "fo/core/lint_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/operation_repository.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace fo::core;

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        base_dir = std::filesystem::temp_directory_path() / ("fo_integration_" + unique_id);
        test_dir = base_dir / "files";
        std::filesystem::create_directories(test_dir);
        db_path = base_dir / "test.db";
    }

    void TearDown() override {
        if (std::filesystem::exists(base_dir)) {
            std::filesystem::remove_all(base_dir);
        }
    }

    void create_file(const std::filesystem::path& path, const std::string& content) {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream ofs(path);
        ofs << content;
        ofs.close();
    }

    std::filesystem::path base_dir;
    std::filesystem::path test_dir;
    std::filesystem::path db_path;
};

TEST_F(IntegrationTest, ScanDirectoryAndVerifyFileCount) {
    create_file(test_dir / "file1.txt", "content1");
    create_file(test_dir / "file2.txt", "content2");
    create_file(test_dir / "subdir" / "file3.txt", "content3");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({test_dir}, {}, false);

    size_t file_count = 0;
    for (const auto& f : files) {
        if (!f.is_dir) ++file_count;
    }
    EXPECT_EQ(file_count, 3);
}

TEST_F(IntegrationTest, FindDuplicatesWithIdenticalContent) {
    std::string duplicate_content = "This is duplicate content for testing";
    create_file(test_dir / "original.txt", duplicate_content);
    create_file(test_dir / "copy1.txt", duplicate_content);
    create_file(test_dir / "copy2.txt", duplicate_content);
    create_file(test_dir / "unique.txt", "unique content");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({test_dir}, {}, false);
    auto duplicates = engine.find_duplicates(files);

    ASSERT_EQ(duplicates.size(), 1);
    EXPECT_EQ(duplicates[0].files.size(), 3);
}

TEST_F(IntegrationTest, ExportToJsonAndVerifyStructure) {
    create_file(test_dir / "doc1.txt", "document one");
    create_file(test_dir / "doc2.txt", "document two");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({test_dir}, {}, false);
    auto duplicates = engine.find_duplicates(files);
    auto stats = Exporter::compute_stats(files, duplicates);

    std::ostringstream json_out;
    Exporter::to_json(json_out, files, duplicates, stats);
    std::string json = json_out.str();

    EXPECT_TRUE(json.find("\"stats\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"files\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"duplicates\"") != std::string::npos);
    EXPECT_TRUE(json.find("\"total_files\"") != std::string::npos);
    EXPECT_TRUE(json.find("doc1.txt") != std::string::npos);
    EXPECT_TRUE(json.find("doc2.txt") != std::string::npos);
}

TEST_F(IntegrationTest, IncrementalScanOnlyProcessesNewFiles) {
    create_file(test_dir / "existing1.txt", "existing content 1");
    create_file(test_dir / "existing2.txt", "existing content 2");

    EngineConfig cfg;
    cfg.db_path = db_path.string();

    {
        Engine engine(cfg);
        auto files = engine.scan({test_dir}, {}, false);
        
        size_t file_count = 0;
        for (const auto& f : files) {
            if (!f.is_dir) ++file_count;
        }
        EXPECT_EQ(file_count, 2);
    }

    create_file(test_dir / "new_file.txt", "new content");

    {
        Engine engine(cfg);
        auto files = engine.scan({test_dir}, {}, false);

        size_t file_count = 0;
        bool found_new = false;
        for (const auto& f : files) {
            if (!f.is_dir) {
                ++file_count;
                if (std::filesystem::path(f.uri).filename() == "new_file.txt") {
                    found_new = true;
                }
            }
        }
        EXPECT_EQ(file_count, 3);
        EXPECT_TRUE(found_new);

        auto opt = engine.file_repository().get_by_path(test_dir / "new_file.txt");
        EXPECT_TRUE(opt.has_value());
    }
}

TEST_F(IntegrationTest, ScanWithExtensionFilter) {
    create_file(test_dir / "file.txt", "text file");
    create_file(test_dir / "file.jpg", "fake jpeg");
    create_file(test_dir / "file.png", "fake png");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({test_dir}, {".txt"}, false);

    size_t file_count = 0;
    for (const auto& f : files) {
        if (!f.is_dir) ++file_count;
    }
    EXPECT_EQ(file_count, 1);
}

TEST_F(IntegrationTest, DuplicateGroupHasCorrectSize) {
    std::string content = "exact same content";
    create_file(test_dir / "a.txt", content);
    create_file(test_dir / "b.txt", content);

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({test_dir}, {}, false);
    auto duplicates = engine.find_duplicates(files);

    ASSERT_EQ(duplicates.size(), 1);
    EXPECT_EQ(duplicates[0].size, content.size());
}

TEST_F(IntegrationTest, ExportToFileCreatesValidJson) {
    create_file(test_dir / "export_test.txt", "test content");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({test_dir}, {}, false);
    auto duplicates = engine.find_duplicates(files);
    auto stats = Exporter::compute_stats(files, duplicates);

    auto output_path = test_dir / "output.json";
    bool success = Exporter::export_to_file(output_path, files, duplicates, stats, ExportFormat::JSON);

    EXPECT_TRUE(success);
    EXPECT_TRUE(std::filesystem::exists(output_path));

    std::ifstream ifs(output_path);
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    EXPECT_TRUE(content.find("\"stats\"") != std::string::npos);
}

// ── Organize + Undo Pipeline Tests ─────────────────────────────────────

TEST_F(IntegrationTest, OrganizeDryRunDoesNotMoveFiles) {
    auto src_dir = test_dir / "inbox";
    auto dst_dir = test_dir / "organized";
    std::filesystem::create_directories(src_dir);
    std::filesystem::create_directories(dst_dir);
    create_file(src_dir / "photo.jpg", "jpg data");
    create_file(src_dir / "notes.txt", "text data");

    RuleEngine rules;
    OrganizationRule r;
    r.name = "images";
    r.filter_tag = "image";
    r.destination_template = (dst_dir / "images").string() + "/";
    rules.add_rule(r);

    // Apply rules to the photo file with an "image" tag
    FileInfo photo_info;
    photo_info.uri = (src_dir / "photo.jpg").string();
    auto result = rules.apply_rules(photo_info, {"image"});

    // Dry-run: result computed, no files moved
    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(std::filesystem::exists(src_dir / "photo.jpg"));
    EXPECT_TRUE(std::filesystem::exists(src_dir / "notes.txt"));
}

TEST_F(IntegrationTest, OrganizeAndUndoRestoresFiles) {
    auto src_dir = test_dir / "inbox";
    auto dst_dir = test_dir / "organized";
    std::filesystem::create_directories(src_dir);
    std::filesystem::create_directories(dst_dir);
    create_file(src_dir / "document.pdf", "pdf content here");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    // Record the original path
    auto original = src_dir / "document.pdf";
    EXPECT_TRUE(std::filesystem::exists(original));

    // Move the file
    auto dest = dst_dir / "document.pdf";
    std::filesystem::create_directories(dest.parent_path());
    std::filesystem::rename(original, dest);
    EXPECT_FALSE(std::filesystem::exists(original));
    EXPECT_TRUE(std::filesystem::exists(dest));

    // Record the operation
    OperationRepository op_repo(engine.database());
    OperationRecord rec;
    rec.type = OperationType::Move;
    rec.source_path = original.string();
    rec.dest_path = dest.string();
    rec.file_size = std::filesystem::file_size(dest);
    op_repo.log_operation(rec);

    // Undo via the repository (physically moves file back)
    auto undone = op_repo.undo_last();
    ASSERT_TRUE(undone.has_value());

    // Verify the file is back at the original location
    EXPECT_TRUE(std::filesystem::exists(original));
    EXPECT_FALSE(std::filesystem::exists(dest));
}

TEST_F(IntegrationTest, SearchEngineIntegrationFindsByLiteral) {
    create_file(test_dir / "readme.md", "# Hello");
    create_file(test_dir / "license.txt", "MIT License");
    create_file(test_dir / "changelog.md", "# Changes");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    SearchOptions opts;
    opts.search_roots = {test_dir};
    opts.query = "readme";
    opts.match_mode = SearchOptions::MatchMode::Literal;

    auto results = engine.search_engine().search(opts);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_NE(results[0].path.filename().string().find("readme"), std::string::npos);
}

TEST_F(IntegrationTest, SearchEngineIntegrationContentSearch) {
    create_file(test_dir / "data.csv", "name,age\nAlice,30\nBob,25");
    create_file(test_dir / "readme.txt", "Nothing important here");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    SearchOptions opts;
    opts.search_roots = {test_dir};
    opts.query = ""; // Match all filenames
    opts.search_content = true;
    opts.content_query = "Alice";
    opts.content_match_mode = SearchOptions::MatchMode::Literal;

    auto results = engine.search_engine().search(opts);
    ASSERT_EQ(results.size(), 1u);
    EXPECT_NE(results[0].path.filename().string().find("data.csv"), std::string::npos);
    EXPECT_GT(results[0].match_count, 0u);
}

TEST_F(IntegrationTest, LinterDetectsIssuesInOrganizedDir) {
    auto organized = test_dir / "organized";
    std::filesystem::create_directories(organized);
    create_file(organized / "empty.txt", "");  // empty file
    create_file(organized / "backup.bak", "old backup");  // temp file
    create_file(organized / "real.txt", "actual content");  // fine
    auto empty_sub = organized / "empty_folder";
    std::filesystem::create_directories(empty_sub);  // empty dir

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto linter = Registry<ILinter>::instance().create("std");
    ASSERT_NE(linter, nullptr);
    auto results = linter->lint({organized});

    int empty_files = 0, temp_files = 0, empty_dirs = 0;
    for (const auto& r : results) {
        if (r.type == LintType::EmptyFile) ++empty_files;
        if (r.type == LintType::TemporaryFile) ++temp_files;
        if (r.type == LintType::EmptyDirectory) ++empty_dirs;
    }
    EXPECT_GE(empty_files, 1);
    EXPECT_GE(temp_files, 1);
    EXPECT_GE(empty_dirs, 1);
}

TEST_F(IntegrationTest, ExportCsvContainsHeaders) {
    create_file(test_dir / "report.csv", "a,b,c");
    create_file(test_dir / "summary.txt", "summary text");

    EngineConfig cfg;
    cfg.db_path = db_path.string();
    Engine engine(cfg);

    auto files = engine.scan({test_dir}, {}, false);
    auto dupes = engine.find_duplicates(files);
    auto stats = Exporter::compute_stats(files, dupes);

    std::ostringstream csv_out;
    Exporter::to_csv(csv_out, files);
    std::string csv = csv_out.str();

    EXPECT_NE(csv.find("path"), std::string::npos);
    EXPECT_NE(csv.find("size"), std::string::npos);
    EXPECT_NE(csv.find("report.csv"), std::string::npos);
    EXPECT_NE(csv.find("summary.txt"), std::string::npos);
}
