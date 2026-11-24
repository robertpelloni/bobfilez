// Operation journal: logging, ordering, and filesystem undo of a rename.
#include <gtest/gtest.h>

#include "fo/core/database.hpp"
#include "fo/core/operation_repository.hpp"

#include <filesystem>
#include <fstream>
#include <memory>

namespace {

struct OperationFixture : ::testing::Test {
    std::filesystem::path dir;
    std::unique_ptr<fo::core::DatabaseManager> db;
    std::unique_ptr<fo::core::OperationRepository> repo;

    void SetUp() override {
        dir = std::filesystem::temp_directory_path() /
              ("bobfilez-ops-test-" + std::to_string(reinterpret_cast<uintptr_t>(this)));
        std::filesystem::create_directories(dir);
        db = std::make_unique<fo::core::DatabaseManager>();
        db->open(dir / "test.db");
        db->migrate();
        repo = std::make_unique<fo::core::OperationRepository>(*db);
    }

    void TearDown() override {
        repo.reset();
        db.reset();
        std::error_code ec;
        std::filesystem::remove_all(dir, ec);
    }

    fo::core::OperationRecord record(fo::core::OperationType type,
                                     const std::string& src, const std::string& dst = "") {
        fo::core::OperationRecord rec;
        rec.timestamp = std::chrono::system_clock::now();
        rec.type = type;
        rec.source_path = src;
        rec.dest_path = dst;
        rec.file_size = 3;
        return rec;
    }
};

TEST_F(OperationFixture, LogAndReadBackMostRecentFirst) {
    repo->log_operation(record(fo::core::OperationType::Move, "a.txt", "b.txt"));
    repo->log_operation(record(fo::core::OperationType::Delete, "c.txt"));

    auto all = repo->get_all(50);
    ASSERT_EQ(all.size(), 2u);
    EXPECT_EQ(all[0].type, fo::core::OperationType::Delete);
    EXPECT_EQ(all[1].type, fo::core::OperationType::Move);
    // Deletes are journaled but not reversible, so only the Move is
    // undoable (a Delete here must not block undoing the Move).
    EXPECT_EQ(repo->get_undoable(50).size(), 1u);
}

TEST_F(OperationFixture, UndoRenameRestoresOriginalPath) {
    auto src = dir / "before.txt";
    auto dst = dir / "after.txt";
    { std::ofstream(src) << "abc"; }
    std::filesystem::rename(src, dst);
    repo->log_operation(record(fo::core::OperationType::Rename, src.string(), dst.string()));

    auto undone = repo->undo_last();
    ASSERT_TRUE(undone.has_value());
    EXPECT_TRUE(std::filesystem::exists(src));
    EXPECT_FALSE(std::filesystem::exists(dst));
}

TEST_F(OperationFixture, UndoLastBatchRestoresWholeBatch) {
    // One command that renames several files journals them under
    // one batch id; a single undo must restore the whole batch,
    // not just the newest file.
    auto batch = repo->allocate_batch_id();
    auto mk = [&](const char* a, const char* b, int64_t bid) {
        auto src = dir / a;
        auto dst = dir / b;
        { std::ofstream(src) << "abc"; }
        std::filesystem::rename(src, dst);
        auto rec = record(fo::core::OperationType::Rename, src.string(), dst.string());
        rec.batch_id = bid;
        repo->log_operation(rec);
        return std::pair{src, dst};
    };
    auto older = mk("old1.txt", "old1-new.txt", batch);
    auto p1 = mk("a.txt", "a-new.txt", batch + 1);
    auto p2 = mk("b.txt", "b-new.txt", batch + 1);

    auto undone = repo->undo_last_batch();
    ASSERT_EQ(undone.size(), 2u);
    EXPECT_TRUE(std::filesystem::exists(p1.first));
    EXPECT_TRUE(std::filesystem::exists(p2.first));
    EXPECT_TRUE(std::filesystem::exists(older.second)); // older batch untouched

    auto undone2 = repo->undo_last_batch();
    ASSERT_EQ(undone2.size(), 1u);
    EXPECT_TRUE(std::filesystem::exists(older.first));
    EXPECT_FALSE(repo->undo_last_batch().size() > 0);
}

TEST_F(OperationFixture, UndoWithEmptyJournalReturnsNullopt) {
    EXPECT_FALSE(repo->undo_last().has_value());
}

} // namespace
