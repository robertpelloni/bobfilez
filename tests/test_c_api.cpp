#include <gtest/gtest.h>

#include "fo/c_api/bobfilez_c_api.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

class CApiTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        base_dir = std::filesystem::temp_directory_path() / ("fo_c_api_" + unique_id);
        test_dir = base_dir / "files";
        std::filesystem::create_directories(test_dir);
    }

    void TearDown() override
    {
        if (std::filesystem::exists(base_dir)) {
            std::filesystem::remove_all(base_dir);
        }
    }

    void create_file(const std::filesystem::path& path, const std::string& content)
    {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream output(path);
        output << content;
    }

    std::filesystem::path base_dir;
    std::filesystem::path test_dir;
};

TEST_F(CApiTest, ScanJsonContainsCreatedFiles)
{
    create_file(test_dir / "one.txt", "alpha");
    create_file(test_dir / "two.txt", "beta");

    char* json = fo_bobfilez_scan_json(test_dir.string().c_str());
    ASSERT_NE(json, nullptr) << fo_bobfilez_last_error();

    std::string text(json);
    fo_bobfilez_free_string(json);

    EXPECT_NE(text.find("one.txt"), std::string::npos);
    EXPECT_NE(text.find("two.txt"), std::string::npos);
    EXPECT_NE(text.find("\"path\""), std::string::npos);
}

TEST_F(CApiTest, DuplicatesJsonContainsDuplicateGroup)
{
    create_file(test_dir / "a.txt", "same-content");
    create_file(test_dir / "b.txt", "same-content");
    create_file(test_dir / "c.txt", "different");

    char* json = fo_bobfilez_duplicates_json(test_dir.string().c_str());
    ASSERT_NE(json, nullptr) << fo_bobfilez_last_error();

    std::string text(json);
    fo_bobfilez_free_string(json);

    EXPECT_NE(text.find("a.txt"), std::string::npos);
    EXPECT_NE(text.find("b.txt"), std::string::npos);
    EXPECT_NE(text.find("\"fast64\""), std::string::npos);
}

TEST_F(CApiTest, StatsJsonContainsCountsAndExtensions)
{
    create_file(test_dir / "alpha.txt", "one");
    create_file(test_dir / "bravo.log", "two");

    char* json = fo_bobfilez_stats_json(test_dir.string().c_str());
    ASSERT_NE(json, nullptr) << fo_bobfilez_last_error();

    std::string text(json);
    fo_bobfilez_free_string(json);

    EXPECT_NE(text.find("\"total_files\""), std::string::npos);
    EXPECT_NE(text.find(".txt"), std::string::npos);
    EXPECT_NE(text.find(".log"), std::string::npos);
}

TEST_F(CApiTest, HashJsonSupportsSingleFileInput)
{
    auto file_path = test_dir / "single.txt";
    create_file(file_path, "hash me");

    char* json = fo_bobfilez_hash_json(file_path.string().c_str());
    ASSERT_NE(json, nullptr) << fo_bobfilez_last_error();

    std::string text(json);
    fo_bobfilez_free_string(json);

    EXPECT_NE(text.find("single.txt"), std::string::npos);
    EXPECT_NE(text.find("\"hash\""), std::string::npos);
}

TEST_F(CApiTest, SummaryTextContainsHumanReadableSections)
{
    create_file(test_dir / "summary_a.txt", "same");
    create_file(test_dir / "summary_b.txt", "same");

    char* scan_summary = fo_bobfilez_scan_summary_text(test_dir.string().c_str());
    ASSERT_NE(scan_summary, nullptr) << fo_bobfilez_last_error();
    std::string scan_text(scan_summary);
    fo_bobfilez_free_string(scan_summary);
    EXPECT_NE(scan_text.find("Scan Summary"), std::string::npos);
    EXPECT_NE(scan_text.find("summary_a.txt"), std::string::npos);

    char* duplicate_summary = fo_bobfilez_duplicates_summary_text(test_dir.string().c_str());
    ASSERT_NE(duplicate_summary, nullptr) << fo_bobfilez_last_error();
    std::string duplicate_text(duplicate_summary);
    fo_bobfilez_free_string(duplicate_summary);
    EXPECT_NE(duplicate_text.find("Duplicate Summary"), std::string::npos);
    EXPECT_NE(duplicate_text.find("summary_b.txt"), std::string::npos);

    char* stats_summary = fo_bobfilez_stats_summary_text(test_dir.string().c_str());
    ASSERT_NE(stats_summary, nullptr) << fo_bobfilez_last_error();
    std::string stats_text(stats_summary);
    fo_bobfilez_free_string(stats_summary);
    EXPECT_NE(stats_text.find("Statistics Summary"), std::string::npos);

    auto file_path = test_dir / "summary_hash.txt";
    create_file(file_path, "hash me please");
    char* hash_summary = fo_bobfilez_hash_summary_text(file_path.string().c_str());
    ASSERT_NE(hash_summary, nullptr) << fo_bobfilez_last_error();
    std::string hash_text(hash_summary);
    fo_bobfilez_free_string(hash_summary);
    EXPECT_NE(hash_text.find("Hash Summary"), std::string::npos);
    EXPECT_NE(hash_text.find("summary_hash.txt"), std::string::npos);
}

TEST(CApiStandaloneTest, NullRootPathSetsError)
{
    char* json = fo_bobfilez_scan_json(nullptr);
    EXPECT_EQ(json, nullptr);
    EXPECT_NE(std::string(fo_bobfilez_last_error()).find("non-empty root path"), std::string::npos);
}
