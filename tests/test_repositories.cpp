#include <gtest/gtest.h>
#include "fo/core/ignore_repository.hpp"
#include "fo/core/duplicate_repository.hpp"
#include "fo/core/file_repository.hpp"
#include "fo/core/database.hpp"
#include <filesystem>
#include <memory>

using namespace fo::core;

// ── IgnoreRepository Tests ──────────────────────────────────────────────

class IgnoreRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        db.open(":memory:");
        db.migrate();
    }

    void TearDown() override {
        // DB closes in destructor
    }

    DatabaseManager db;
};

TEST_F(IgnoreRepositoryTest, AddAndGetAll) {
    IgnoreRepository repo(db);
    repo.add("*.tmp", "Temporary files");
    repo.add("*.bak", "Backup files");

    auto rules = repo.get_all();
    ASSERT_EQ(rules.size(), 2u);
    EXPECT_EQ(rules[0].pattern, "*.tmp");
    EXPECT_EQ(rules[1].pattern, "*.bak");
}

TEST_F(IgnoreRepositoryTest, RemovePattern) {
    IgnoreRepository repo(db);
    repo.add("*.tmp", "Temporary");
    repo.add("*.log", "Logs");

    repo.remove("*.tmp");
    auto rules = repo.get_all();
    ASSERT_EQ(rules.size(), 1u);
    EXPECT_EQ(rules[0].pattern, "*.log");
}

TEST_F(IgnoreRepositoryTest, RemoveNonExistentIsNoop) {
    IgnoreRepository repo(db);
    repo.add("*.tmp", "Temp");
    EXPECT_NO_THROW(repo.remove("nonexistent"));
    EXPECT_EQ(repo.get_all().size(), 1u);
}

TEST_F(IgnoreRepositoryTest, IsIgnoredSimplePattern) {
    IgnoreRepository repo(db);
    repo.add(R"(.*\.tmp)", "Temporary files");

    EXPECT_TRUE(repo.is_ignored("test.tmp"));
    EXPECT_TRUE(repo.is_ignored("path/to/file.tmp"));
    EXPECT_FALSE(repo.is_ignored("test.txt"));
}

TEST_F(IgnoreRepositoryTest, IsIgnoredMultiplePatterns) {
    IgnoreRepository repo(db);
    repo.add(R"(.*\.tmp)", "Temp");
    repo.add(R"(.*\.bak)", "Backup");
    repo.add("Thumbs.db", "System file");

    EXPECT_TRUE(repo.is_ignored("file.tmp"));
    EXPECT_TRUE(repo.is_ignored("file.bak"));
    EXPECT_TRUE(repo.is_ignored("Thumbs.db"));
    EXPECT_FALSE(repo.is_ignored("file.txt"));
    EXPECT_FALSE(repo.is_ignored("important.doc"));
}

TEST_F(IgnoreRepositoryTest, EmptyRepositoryReturnsNothing) {
    IgnoreRepository repo(db);
    EXPECT_TRUE(repo.get_all().empty());
    EXPECT_FALSE(repo.is_ignored("anything"));
}

TEST_F(IgnoreRepositoryTest, DuplicatePatternIsSilentlyIgnored) {
    IgnoreRepository repo(db);
    repo.add("*.tmp", "First");
    // The table has UNIQUE on pattern, so this should not crash
    // It may silently fail or throw — either is acceptable
    EXPECT_NO_THROW(repo.add("*.tmp", "Second"));
}

// ── DuplicateRepository Tests ──────────────────────────────────────────

class DuplicateRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        db.open(":memory:");
        db.migrate();
        file_repo = std::make_unique<FileRepository>(db);
        dup_repo = std::make_unique<DuplicateRepository>(db);
    }

    void TearDown() override {
        dup_repo.reset();
        file_repo.reset();
    }

    DatabaseManager db;
    std::unique_ptr<FileRepository> file_repo;
    std::unique_ptr<DuplicateRepository> dup_repo;
};

TEST_F(DuplicateRepositoryTest, CreateAndGetGroup) {
    // Insert files first
    FileInfo f1; f1.uri = "/a.txt"; f1.size = 100; f1.is_dir = false;
    FileInfo f2; f2.uri = "/b.txt"; f2.size = 100; f2.is_dir = false;

    file_repo->upsert(f1);
    file_repo->upsert(f2);

    // Create a duplicate group with primary file
    int64_t group_id = dup_repo->create_group(f1.id);
    EXPECT_GT(group_id, 0);

    dup_repo->add_member(group_id, f1.id);
    dup_repo->add_member(group_id, f2.id);

    auto groups = dup_repo->get_all_groups();
    ASSERT_FALSE(groups.empty());
    EXPECT_EQ(groups[0].primary_file_id, f1.id);
}

TEST_F(DuplicateRepositoryTest, GetMembersOfGroup) {
    FileInfo f1; f1.uri = "/x.txt"; f1.size = 50; f1.is_dir = false;
    FileInfo f2; f2.uri = "/y.txt"; f2.size = 50; f2.is_dir = false;

    file_repo->upsert(f1);
    file_repo->upsert(f2);

    int64_t group_id = dup_repo->create_group(f1.id);
    dup_repo->add_member(group_id, f1.id);
    dup_repo->add_member(group_id, f2.id);

    auto groups = dup_repo->get_all_groups();
    ASSERT_EQ(groups.size(), 1u);
    // Check that the group has the members
    EXPECT_EQ(groups[0].member_ids.size(), 2u);
}

TEST_F(DuplicateRepositoryTest, EmptyGroupsOnFreshDb) {
    auto groups = dup_repo->get_all_groups();
    EXPECT_TRUE(groups.empty());
}

TEST_F(DuplicateRepositoryTest, ClearAllRemovesEverything) {
    FileInfo f1; f1.uri = "/z.txt"; f1.size = 10; f1.is_dir = false;
    file_repo->upsert(f1);

    int64_t group_id = dup_repo->create_group(f1.id);
    dup_repo->add_member(group_id, f1.id);

    EXPECT_EQ(dup_repo->get_all_groups().size(), 1u);

    dup_repo->clear_all();

    auto groups = dup_repo->get_all_groups();
    EXPECT_TRUE(groups.empty());
}
