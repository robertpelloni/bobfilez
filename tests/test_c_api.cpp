#include <gtest/gtest.h>

#include "fo/c_api/bobfilez_c_api.h"
#include "fo/core/engine.hpp"
#include "fo/core/operation_repository.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

namespace {

class ScopedEnvVar {
public:
    ScopedEnvVar(const char* name, const std::string& value) : name_(name)
    {
        const char* existing = std::getenv(name_);
        if (existing != nullptr) {
            old_value_ = existing;
        }
        had_old_value_ = existing != nullptr;
        set(value);
    }

    ~ScopedEnvVar()
    {
        if (had_old_value_ && old_value_.has_value()) {
            set(*old_value_);
        } else {
            clear();
        }
    }

private:
    void set(const std::string& value)
    {
#ifdef _WIN32
        _putenv_s(name_, value.c_str());
#else
        setenv(name_, value.c_str(), 1);
#endif
    }

    void clear()
    {
#ifdef _WIN32
        _putenv_s(name_, "");
#else
        unsetenv(name_);
#endif
    }

    const char* name_;
    bool had_old_value_ = false;
    std::optional<std::string> old_value_;
};

} // namespace

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

TEST_F(CApiTest, LintJsonAndSummaryExposeDetectedIssues)
{
    auto empty_file = test_dir / "empty.txt";
    std::ofstream(empty_file).close();
    auto temp_file = test_dir / "cache.bak";
    create_file(temp_file, "temporary");

    char* json = fo_bobfilez_lint_json(test_dir.string().c_str());
    ASSERT_NE(json, nullptr) << fo_bobfilez_last_error();
    std::string json_text(json);
    fo_bobfilez_free_string(json);
    EXPECT_NE(json_text.find("EmptyFile"), std::string::npos);
    EXPECT_NE(json_text.find("TemporaryFile"), std::string::npos);

    char* summary = fo_bobfilez_lint_summary_text(test_dir.string().c_str());
    ASSERT_NE(summary, nullptr) << fo_bobfilez_last_error();
    std::string summary_text(summary);
    fo_bobfilez_free_string(summary);
    EXPECT_NE(summary_text.find("Lint Summary"), std::string::npos);
    EXPECT_NE(summary_text.find("empty.txt"), std::string::npos);
}

TEST_F(CApiTest, HistoryAndIgnoreExposeDatabaseBackedState)
{
    const auto db_path = (base_dir / "c_api_state.db").string();
    ScopedEnvVar db_override("BOBFILEZ_DB_PATH", db_path);

    fo::core::EngineConfig config;
    config.db_path = db_path;
    fo::core::Engine engine(config);

    engine.ignore_repository().add("thumbs.db", "Windows thumbnail cache");
    engine.ignore_repository().add("*.tmp", "temporary files");

    fo::core::OperationRepository op_repo(engine.database());
    fo::core::OperationRecord record;
    record.timestamp = std::chrono::system_clock::now();
    record.type = fo::core::OperationType::Rename;
    record.source_path = (test_dir / "before.txt").string();
    record.dest_path = (test_dir / "after.txt").string();
    op_repo.log_operation(record);

    char* history_json = fo_bobfilez_history_json("");
    ASSERT_NE(history_json, nullptr) << fo_bobfilez_last_error();
    std::string history_json_text(history_json);
    fo_bobfilez_free_string(history_json);
    EXPECT_NE(history_json_text.find("\"type\": \"rename\""), std::string::npos);
    EXPECT_NE(history_json_text.find("after.txt"), std::string::npos);

    char* history_summary = fo_bobfilez_history_summary_text("");
    ASSERT_NE(history_summary, nullptr) << fo_bobfilez_last_error();
    std::string history_summary_text(history_summary);
    fo_bobfilez_free_string(history_summary);
    EXPECT_NE(history_summary_text.find("History Summary"), std::string::npos);
    EXPECT_NE(history_summary_text.find("before.txt"), std::string::npos);

    char* ignore_json = fo_bobfilez_ignore_json("");
    ASSERT_NE(ignore_json, nullptr) << fo_bobfilez_last_error();
    std::string ignore_json_text(ignore_json);
    fo_bobfilez_free_string(ignore_json);
    EXPECT_NE(ignore_json_text.find("thumbs.db"), std::string::npos);
    EXPECT_NE(ignore_json_text.find("*.tmp"), std::string::npos);

    char* ignore_summary = fo_bobfilez_ignore_summary_text("");
    ASSERT_NE(ignore_summary, nullptr) << fo_bobfilez_last_error();
    std::string ignore_summary_text(ignore_summary);
    fo_bobfilez_free_string(ignore_summary);
    EXPECT_NE(ignore_summary_text.find("Ignore Rules Summary"), std::string::npos);
    EXPECT_NE(ignore_summary_text.find("Windows thumbnail cache"), std::string::npos);
}

TEST_F(CApiTest, IgnoreAddAndRemoveActionsUpdateDatabaseBackedState)
{
    const auto db_path = (base_dir / "c_api_ignore_actions.db").string();
    ScopedEnvVar db_override("BOBFILEZ_DB_PATH", db_path);

    char* add_json = fo_bobfilez_ignore_add_json("desktop.ini", "Windows metadata file");
    ASSERT_NE(add_json, nullptr) << fo_bobfilez_last_error();
    std::string add_json_text(add_json);
    fo_bobfilez_free_string(add_json);
    EXPECT_NE(add_json_text.find("\"action\": \"add\""), std::string::npos);
    EXPECT_NE(add_json_text.find("desktop.ini"), std::string::npos);

    char* add_summary = fo_bobfilez_ignore_add_summary_text("*.cache", "cache files");
    ASSERT_NE(add_summary, nullptr) << fo_bobfilez_last_error();
    std::string add_summary_text(add_summary);
    fo_bobfilez_free_string(add_summary);
    EXPECT_NE(add_summary_text.find("Ignore Rule Added"), std::string::npos);
    EXPECT_NE(add_summary_text.find("*.cache"), std::string::npos);

    char* ignore_json = fo_bobfilez_ignore_json("");
    ASSERT_NE(ignore_json, nullptr) << fo_bobfilez_last_error();
    std::string ignore_json_text(ignore_json);
    fo_bobfilez_free_string(ignore_json);
    EXPECT_NE(ignore_json_text.find("desktop.ini"), std::string::npos);
    EXPECT_NE(ignore_json_text.find("*.cache"), std::string::npos);

    char* remove_summary = fo_bobfilez_ignore_remove_summary_text("desktop.ini");
    ASSERT_NE(remove_summary, nullptr) << fo_bobfilez_last_error();
    std::string remove_summary_text(remove_summary);
    fo_bobfilez_free_string(remove_summary);
    EXPECT_NE(remove_summary_text.find("Ignore Rule Removed"), std::string::npos);
    EXPECT_NE(remove_summary_text.find("desktop.ini"), std::string::npos);

    char* after_remove_json = fo_bobfilez_ignore_json("");
    ASSERT_NE(after_remove_json, nullptr) << fo_bobfilez_last_error();
    std::string after_remove_json_text(after_remove_json);
    fo_bobfilez_free_string(after_remove_json);
    EXPECT_EQ(after_remove_json_text.find("desktop.ini"), std::string::npos);
    EXPECT_NE(after_remove_json_text.find("*.cache"), std::string::npos);
}

TEST(CApiStandaloneTest, NullRootPathSetsError)
{
    char* json = fo_bobfilez_scan_json(nullptr);
    EXPECT_EQ(json, nullptr);
    EXPECT_NE(std::string(fo_bobfilez_last_error()).find("non-empty root path"), std::string::npos);
}
