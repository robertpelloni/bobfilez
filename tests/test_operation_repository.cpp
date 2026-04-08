#include <gtest/gtest.h>
#include "fo/core/operation_repository.hpp"
#include "fo/core/database.hpp"
#include <filesystem>
#include <fstream>
#include <chrono>

using namespace fo::core;

class OperationRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        db.open(":memory:");
        db.migrate();
        repo = std::make_unique<OperationRepository>(db);
    }

    void TearDown() override {
        repo.reset();
    }

    void create_file(const std::filesystem::path& path, const std::string& content = "data") {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream ofs(path);
        ofs << content;
    }

    DatabaseManager db;
    std::unique_ptr<OperationRepository> repo;
};

// ── Log + Retrieve Tests ─────────────────────────────────────────────────

TEST_F(OperationRepositoryTest, LogOperationReturnsId) {
    OperationRecord rec;
    rec.timestamp = std::chrono::system_clock::now();
    rec.type = OperationType::Move;
    rec.source_path = "/src/file.txt";
    rec.dest_path = "/dst/file.txt";
    rec.file_size = 1024;
    rec.status = "completed";

    int64_t id = repo->log_operation(rec);
    EXPECT_GT(id, 0);
}

TEST_F(OperationRepositoryTest, GetAllReturnsLoggedOps) {
    OperationRecord rec;
    rec.timestamp = std::chrono::system_clock::now();
    rec.type = OperationType::Copy;
    rec.source_path = "/src/a.txt";
    rec.dest_path = "/dst/a.txt";
    rec.status = "completed";

    repo->log_operation(rec);
    auto all = repo->get_all();
    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].source_path, "/src/a.txt");
    EXPECT_EQ(all[0].dest_path, "/dst/a.txt");
}

TEST_F(OperationRepositoryTest, GetAllOrderedByTimestampDesc) {
    for (int i = 0; i < 3; ++i) {
        OperationRecord rec;
        rec.timestamp = std::chrono::system_clock::now() + std::chrono::seconds(i);
        rec.type = OperationType::Move;
        rec.source_path = "/src/" + std::to_string(i);
        rec.dest_path = "/dst/" + std::to_string(i);
        rec.status = "completed";
        repo->log_operation(rec);
    }

    auto all = repo->get_all();
    ASSERT_EQ(all.size(), 3u);
    // Most recent first
    EXPECT_GT(all[0].source_path, all[1].source_path);
    EXPECT_GT(all[1].source_path, all[2].source_path);
}

TEST_F(OperationRepositoryTest, GetAllRespectsLimit) {
    for (int i = 0; i < 10; ++i) {
        OperationRecord rec;
        rec.timestamp = std::chrono::system_clock::now() + std::chrono::seconds(i);
        rec.type = OperationType::Copy;
        rec.source_path = "/src/" + std::to_string(i);
        rec.dest_path = "/dst/" + std::to_string(i);
        rec.status = "completed";
        repo->log_operation(rec);
    }

    auto limited = repo->get_all(3);
    EXPECT_EQ(limited.size(), 3u);
}

TEST_F(OperationRepositoryTest, GetAllReturnsEmptyForFreshDb) {
    auto all = repo->get_all();
    EXPECT_TRUE(all.empty());
}

// ── Operation Types ──────────────────────────────────────────────────────

TEST_F(OperationRepositoryTest, AllOperationTypes) {
    OperationType types[] = {OperationType::Move, OperationType::Copy,
                             OperationType::Delete, OperationType::Rename};
    const char* names[] = {"move", "copy", "delete", "rename"};

    for (int i = 0; i < 4; ++i) {
        OperationRecord rec;
        rec.timestamp = std::chrono::system_clock::now() + std::chrono::seconds(i);
        rec.type = types[i];
        rec.source_path = "/src/file";
        rec.dest_path = "/dst/file";
        rec.status = "completed";
        repo->log_operation(rec);
    }

    auto all = repo->get_all(4);
    ASSERT_EQ(all.size(), 4u);
    // All should have been stored and retrieved (order is DESC)
    std::set<OperationType> result_types;
    for (const auto& r : all) {
        result_types.insert(r.type);
    }
    EXPECT_EQ(result_types.size(), 4u);
}

// ── GetById Tests ────────────────────────────────────────────────────────

TEST_F(OperationRepositoryTest, GetByIdReturnsCorrectRecord) {
    OperationRecord rec;
    rec.timestamp = std::chrono::system_clock::now();
    rec.type = OperationType::Rename;
    rec.source_path = "/old_name.txt";
    rec.dest_path = "/new_name.txt";
    rec.file_size = 2048;
    rec.file_hash = "abc123";
    rec.status = "completed";

    int64_t id = repo->log_operation(rec);
    auto found = repo->get_by_id(id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->id, id);
    EXPECT_EQ(found->source_path, "/old_name.txt");
    EXPECT_EQ(found->dest_path, "/new_name.txt");
    EXPECT_EQ(found->file_size, 2048);
    EXPECT_EQ(found->file_hash, "abc123");
}

TEST_F(OperationRepositoryTest, GetByIdReturnsNulloptForMissing) {
    auto found = repo->get_by_id(99999);
    EXPECT_FALSE(found.has_value());
}

// ── Undoable Tests ──────────────────────────────────────────────────────

TEST_F(OperationRepositoryTest, GetUndoableReturnsOnlyNonUndone) {
    // Log two operations
    OperationRecord rec1, rec2;
    rec1.timestamp = std::chrono::system_clock::now();
    rec1.type = OperationType::Move;
    rec1.source_path = "/src/a.txt";
    rec1.dest_path = "/dst/a.txt";
    rec1.status = "completed";

    rec2.timestamp = std::chrono::system_clock::now() + std::chrono::seconds(1);
    rec2.type = OperationType::Copy;
    rec2.source_path = "/src/b.txt";
    rec2.dest_path = "/dst/b.txt";
    rec2.status = "completed";

    int64_t id1 = repo->log_operation(rec1);
    repo->log_operation(rec2);

    // Both should be undoable
    auto undoable = repo->get_undoable();
    EXPECT_EQ(undoable.size(), 2u);

    // Mark first as undone
    repo->mark_undone(id1);

    // Only one should be undoable now
    undoable = repo->get_undoable();
    EXPECT_EQ(undoable.size(), 1u);
    EXPECT_EQ(undoable[0].source_path, "/src/b.txt");
}

TEST_F(OperationRepositoryTest, GetUndoableExcludesNonCompleted) {
    OperationRecord rec;
    rec.timestamp = std::chrono::system_clock::now();
    rec.type = OperationType::Move;
    rec.source_path = "/src/file";
    rec.dest_path = "/dst/file";
    rec.status = "failed";  // Not completed

    repo->log_operation(rec);
    auto undoable = repo->get_undoable();
    EXPECT_TRUE(undoable.empty());
}

// ── Undo Tests ───────────────────────────────────────────────────────────

TEST_F(OperationRepositoryTest, UndoLastMoveOperation) {
    auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    auto test_dir = std::filesystem::temp_directory_path() / ("fo_test_op_" + unique_id);
    auto src = test_dir / "src.txt";
    auto dst = test_dir / "dst.txt";
    create_file(src, "undo test");

    // Move the file
    std::filesystem::rename(src, dst);
    ASSERT_TRUE(std::filesystem::exists(dst));
    ASSERT_FALSE(std::filesystem::exists(src));

    // Log the operation
    OperationRecord rec;
    rec.timestamp = std::chrono::system_clock::now();
    rec.type = OperationType::Move;
    rec.source_path = src.string();
    rec.dest_path = dst.string();
    rec.status = "completed";
    repo->log_operation(rec);

    // Undo it
    auto undone = repo->undo_last();
    ASSERT_TRUE(undone.has_value());
    EXPECT_TRUE(std::filesystem::exists(src));
    EXPECT_FALSE(std::filesystem::exists(dst));

    std::filesystem::remove_all(test_dir);
}

TEST_F(OperationRepositoryTest, UndoLastCopyOperation) {
    auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    auto test_dir = std::filesystem::temp_directory_path() / ("fo_test_op_" + unique_id);
    auto src = test_dir / "src.txt";
    auto dst = test_dir / "dst.txt";
    create_file(src, "undo copy test");

    // Copy the file
    std::filesystem::copy_file(src, dst);
    ASSERT_TRUE(std::filesystem::exists(src));
    ASSERT_TRUE(std::filesystem::exists(dst));

    // Log the operation
    OperationRecord rec;
    rec.timestamp = std::chrono::system_clock::now();
    rec.type = OperationType::Copy;
    rec.source_path = src.string();
    rec.dest_path = dst.string();
    rec.status = "completed";
    repo->log_operation(rec);

    // Undo it (should delete the copy)
    auto undone = repo->undo_last();
    ASSERT_TRUE(undone.has_value());
    EXPECT_TRUE(std::filesystem::exists(src));
    EXPECT_FALSE(std::filesystem::exists(dst));

    std::filesystem::remove_all(test_dir);
}

TEST_F(OperationRepositoryTest, UndoLastReturnsNulloptWhenNoneUndoable) {
    auto undone = repo->undo_last();
    EXPECT_FALSE(undone.has_value());
}

// ── Clear Old Tests ──────────────────────────────────────────────────────

TEST_F(OperationRepositoryTest, ClearOldRemovesOldRecords) {
    // Old record (31 days ago)
    OperationRecord old_rec;
    old_rec.timestamp = std::chrono::system_clock::now() - std::chrono::hours(24 * 31);
    old_rec.type = OperationType::Move;
    old_rec.source_path = "/old/file";
    old_rec.dest_path = "/old/dest";
    old_rec.status = "completed";
    repo->log_operation(old_rec);

    // Recent record
    OperationRecord new_rec;
    new_rec.timestamp = std::chrono::system_clock::now();
    new_rec.type = OperationType::Copy;
    new_rec.source_path = "/new/file";
    new_rec.dest_path = "/new/dest";
    new_rec.status = "completed";
    repo->log_operation(new_rec);

    auto all = repo->get_all(1000);
    EXPECT_EQ(all.size(), 2u);

    repo->clear_old(30);

    all = repo->get_all(1000);
    EXPECT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].source_path, "/new/file");
}

// ── Field Integrity Tests ────────────────────────────────────────────────

TEST_F(OperationRepositoryTest, FileSizeAndHashPreserved) {
    OperationRecord rec;
    rec.timestamp = std::chrono::system_clock::now();
    rec.type = OperationType::Move;
    rec.source_path = "/src/big.bin";
    rec.dest_path = "/dst/big.bin";
    rec.file_size = 1073741824; // 1 GB
    rec.file_hash = "sha256:abcdef1234567890";
    rec.status = "completed";

    int64_t id = repo->log_operation(rec);
    auto found = repo->get_by_id(id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->file_size, 1073741824);
    EXPECT_EQ(found->file_hash, "sha256:abcdef1234567890");
}

TEST_F(OperationRepositoryTest, UndoneFlagPreserved) {
    OperationRecord rec;
    rec.timestamp = std::chrono::system_clock::now();
    rec.type = OperationType::Copy;
    rec.source_path = "/src/file";
    rec.dest_path = "/dst/file";
    rec.status = "completed";
    rec.undone = true; // Log it as already undone

    int64_t id = repo->log_operation(rec);
    auto found = repo->get_by_id(id);
    ASSERT_TRUE(found.has_value());
    EXPECT_TRUE(found->undone);
}
