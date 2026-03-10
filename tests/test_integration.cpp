#include <gtest/gtest.h>
#include "fo/core/engine.hpp"
#include "fo/core/export.hpp"
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
