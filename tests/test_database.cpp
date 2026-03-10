#include <gtest/gtest.h>
#include "fo/core/database.hpp"
#include "fo/core/file_repository.hpp"
#include "fo/core/types.hpp"
#include <filesystem>
#include <fstream>

using namespace fo::core;

class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "fo_db_test";
        std::filesystem::create_directories(test_dir);
        db_path = test_dir / "test.db";
    }

    void TearDown() override {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    std::filesystem::path test_dir;
    std::filesystem::path db_path;
};

TEST_F(DatabaseTest, OpenCreatesDatabase) {
    DatabaseManager db;
    db.open(db_path);
    
    EXPECT_TRUE(std::filesystem::exists(db_path));
    EXPECT_NE(db.get_db(), nullptr);
    
    db.close();
}

TEST_F(DatabaseTest, OpenInMemory) {
    DatabaseManager db;
    db.open(":memory:");
    
    EXPECT_NE(db.get_db(), nullptr);
    
    db.close();
}

TEST_F(DatabaseTest, MigrateCreatesSchema) {
    DatabaseManager db;
    db.open(db_path);
    db.migrate();
    
    int table_count = db.query_int(
        "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='files'"
    );
    EXPECT_EQ(table_count, 1);
    
    db.close();
}

TEST_F(DatabaseTest, ExecuteSimpleStatement) {
    DatabaseManager db;
    db.open(":memory:");
    db.migrate();
    
    EXPECT_NO_THROW(db.execute("SELECT 1"));
    
    db.close();
}

TEST_F(DatabaseTest, QueryIntReturnsValue) {
    DatabaseManager db;
    db.open(":memory:");
    
    int result = db.query_int("SELECT 42");
    EXPECT_EQ(result, 42);
    
    db.close();
}

class FileRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir = std::filesystem::temp_directory_path() / "fo_repo_test";
        std::filesystem::create_directories(test_dir);
        
        db = std::make_unique<DatabaseManager>();
        db->open(":memory:");
        db->migrate();
        
        repo = std::make_unique<FileRepository>(*db);
    }

    void TearDown() override {
        repo.reset();
        db->close();
        db.reset();
        
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    FileInfo create_test_file(const std::string& name, std::uintmax_t size = 100) {
        FileInfo info;
        info.uri = (test_dir / name).string();
        info.size = size;
        info.mtime = std::chrono::file_clock::now();
        info.is_dir = false;
        return info;
    }

    std::filesystem::path test_dir;
    std::unique_ptr<DatabaseManager> db;
    std::unique_ptr<FileRepository> repo;
};

TEST_F(FileRepositoryTest, UpsertNewFile) {
    FileInfo file = create_test_file("test.txt");
    
    UpsertResult result = repo->upsert(file);
    
    EXPECT_TRUE(result.is_new);
    EXPECT_FALSE(result.is_modified);
    EXPECT_GT(file.id, 0);
}

TEST_F(FileRepositoryTest, UpsertExistingFileNoChange) {
    FileInfo file = create_test_file("test.txt");
    repo->upsert(file);
    int64_t original_id = file.id;
    
    UpsertResult result = repo->upsert(file);
    
    EXPECT_FALSE(result.is_new);
    EXPECT_FALSE(result.is_modified);
    EXPECT_EQ(file.id, original_id);
}

TEST_F(FileRepositoryTest, UpsertExistingFileModified) {
    FileInfo file = create_test_file("test.txt", 100);
    repo->upsert(file);
    int64_t original_id = file.id;
    
    file.size = 200;
    UpsertResult result = repo->upsert(file);
    
    EXPECT_FALSE(result.is_new);
    EXPECT_TRUE(result.is_modified);
    EXPECT_EQ(file.id, original_id);
}

TEST_F(FileRepositoryTest, GetByPath) {
    FileInfo file = create_test_file("find_me.txt", 500);
    repo->upsert(file);
    
    auto found = repo->get_by_path(test_dir / "find_me.txt");
    
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->id, file.id);
    EXPECT_EQ(found->size, 500);
}

TEST_F(FileRepositoryTest, GetByPathNotFound) {
    auto found = repo->get_by_path(test_dir / "nonexistent.txt");
    
    EXPECT_FALSE(found.has_value());
}

TEST_F(FileRepositoryTest, GetById) {
    FileInfo file = create_test_file("by_id.txt", 300);
    repo->upsert(file);
    
    auto found = repo->get_by_id(file.id);
    
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->size, 300);
}

TEST_F(FileRepositoryTest, GetByIdNotFound) {
    auto found = repo->get_by_id(99999);
    
    EXPECT_FALSE(found.has_value());
}

TEST_F(FileRepositoryTest, DeleteFiles) {
    FileInfo file1 = create_test_file("delete1.txt");
    FileInfo file2 = create_test_file("delete2.txt");
    repo->upsert(file1);
    repo->upsert(file2);
    
    repo->delete_files({file1.id});
    
    EXPECT_FALSE(repo->get_by_id(file1.id).has_value());
    EXPECT_TRUE(repo->get_by_id(file2.id).has_value());
}

TEST_F(FileRepositoryTest, UpdatePath) {
    FileInfo file = create_test_file("original.txt");
    repo->upsert(file);
    
    auto new_path = test_dir / "renamed.txt";
    repo->update_path(file.id, new_path);
    
    auto found = repo->get_by_id(file.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->uri, new_path.string());
}

TEST_F(FileRepositoryTest, AddAndGetHash) {
    FileInfo file = create_test_file("hashed.txt");
    repo->upsert(file);
    
    repo->add_hash(file.id, "sha256", "abc123def456");
    repo->add_hash(file.id, "fast64", "xyz789");
    
    auto hashes = repo->get_hashes(file.id);
    
    EXPECT_EQ(hashes.size(), 2);
    
    bool found_sha = false, found_fast = false;
    for (const auto& [algo, value] : hashes) {
        if (algo == "sha256" && value == "abc123def456") found_sha = true;
        if (algo == "fast64" && value == "xyz789") found_fast = true;
    }
    EXPECT_TRUE(found_sha);
    EXPECT_TRUE(found_fast);
}

TEST_F(FileRepositoryTest, AddAndGetTags) {
    FileInfo file = create_test_file("tagged.txt");
    repo->upsert(file);
    
    repo->add_tag(file.id, "landscape", 0.95, "ai");
    repo->add_tag(file.id, "favorite", 1.0, "user");
    
    auto tags = repo->get_tags(file.id);
    
    EXPECT_EQ(tags.size(), 2);
    
    bool found_landscape = false, found_favorite = false;
    for (const auto& [tag, conf] : tags) {
        if (tag == "landscape" && conf == 0.95) found_landscape = true;
        if (tag == "favorite" && conf == 1.0) found_favorite = true;
    }
    EXPECT_TRUE(found_landscape);
    EXPECT_TRUE(found_favorite);
}

TEST_F(FileRepositoryTest, InsertMultipleFiles) {
    std::vector<FileInfo> files;
    for (int i = 0; i < 10; ++i) {
        files.push_back(create_test_file("file" + std::to_string(i) + ".txt", i * 100));
    }
    
    for (auto& file : files) {
        repo->upsert(file);
        EXPECT_GT(file.id, 0);
    }
    
    for (const auto& file : files) {
        auto found = repo->get_by_id(file.id);
        ASSERT_TRUE(found.has_value());
        EXPECT_EQ(found->size, file.size);
    }
}

TEST_F(FileRepositoryTest, DirectoryFlag) {
    FileInfo dir;
    dir.uri = (test_dir / "subdir").string();
    dir.size = 0;
    dir.mtime = std::chrono::file_clock::now();
    dir.is_dir = true;
    
    repo->upsert(dir);
    
    auto found = repo->get_by_id(dir.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_TRUE(found->is_dir);
}
