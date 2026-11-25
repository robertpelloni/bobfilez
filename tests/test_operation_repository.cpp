// Operation journal: logging, ordering, and filesystem undo of a rename.
#include <gtest/gtest.h>

#include "fo/core/database.hpp"
#include "fo/core/interfaces.hpp"
#include "fo/core/operation_repository.hpp"
#include "fo/core/provider_registration.hpp"
#include "fo/core/registry.hpp"

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
    ASSERT_EQ(undone.undone.size(), 2u);
    EXPECT_TRUE(std::filesystem::exists(p1.first));
    EXPECT_TRUE(std::filesystem::exists(p2.first));
    EXPECT_TRUE(std::filesystem::exists(older.second)); // older batch untouched

    auto undone2 = repo->undo_last_batch();
    ASSERT_EQ(undone2.undone.size(), 1u);
    EXPECT_TRUE(std::filesystem::exists(older.first));
    EXPECT_TRUE(repo->undo_last_batch().undone.empty());
}

TEST_F(OperationFixture, UndoBatchOver500UndoneInOneCall) {
    // Muse review: get_undoable(500) truncated big batches, so
    // one undo of a 520-file organize restored only 500 and
    // reported success. The batch must come back whole.
    auto batch = repo->allocate_batch_id();
    const int n = 520;
    for (int i = 0; i < n; ++i) {
        auto src = dir / ("f" + std::to_string(i) + ".txt");
        auto dst = dir / ("f" + std::to_string(i) + ".new");
        { std::ofstream(src) << "abc"; } // 3 bytes = record()'s file_size
        std::filesystem::rename(src, dst);
        auto rec = record(fo::core::OperationType::Rename, src.string(), dst.string());
        rec.batch_id = batch;
        repo->log_operation(rec);
    }
    auto outcome = repo->undo_last_batch();
    EXPECT_EQ(outcome.undone.size(), static_cast<size_t>(n));
    EXPECT_TRUE(outcome.skipped.empty());
    EXPECT_TRUE(std::filesystem::exists(dir / "f0.txt"));
    EXPECT_TRUE(std::filesystem::exists(dir / "f519.txt"));
}

TEST_F(OperationFixture, UndoRefusesDeleteBatchAndDismissUnblocks) {
    // Muse review: after delete-duplicates, undo used to
    // silently reverse the *previous* batch. It must refuse the
    // irreversible delete batch; only an explicit dismiss may
    // move past it.
    auto move_src = dir / "keep.txt";
    auto move_dst = dir / "keep-moved.txt";
    { std::ofstream(move_src) << "abc"; }
    std::filesystem::rename(move_src, move_dst);
    auto mv = record(fo::core::OperationType::Rename, move_src.string(), move_dst.string());
    mv.batch_id = repo->allocate_batch_id();
    repo->log_operation(mv);

    auto del = record(fo::core::OperationType::Delete, (dir / "dup.txt").string());
    del.batch_id = repo->allocate_batch_id();
    repo->log_operation(del);

    auto outcome = repo->undo_last_batch();
    EXPECT_TRUE(outcome.refused_delete_batch);
    EXPECT_EQ(outcome.delete_batch_records, 1);
    EXPECT_TRUE(outcome.undone.empty());
    EXPECT_TRUE(std::filesystem::exists(move_dst)); // older batch untouched

    EXPECT_EQ(repo->dismiss_newest_batch(), 1);
    auto outcome2 = repo->undo_last_batch();
    ASSERT_EQ(outcome2.undone.size(), 1u);
    EXPECT_TRUE(std::filesystem::exists(move_src));
}

TEST_F(OperationFixture, UndoRefusesTamperedDestination) {
    // Muse review: undo moved back whatever sat at dest. A file
    // modified after the move (size differs from the journal)
    // must be refused and reported, never overwritten/moved.
    auto src = dir / "orig.txt";
    auto dst = dir / "moved.txt";
    { std::ofstream(src) << "abc"; }
    std::filesystem::rename(src, dst);
    repo->log_operation(record(fo::core::OperationType::Rename, src.string(), dst.string()));
    { std::ofstream(dst, std::ios::app) << "tampered"; }

    auto outcome = repo->undo_last_batch();
    EXPECT_TRUE(outcome.undone.empty());
    ASSERT_EQ(outcome.skipped.size(), 1u);
    EXPECT_NE(outcome.skipped[0].second.find("changed"), std::string::npos);
    EXPECT_TRUE(std::filesystem::exists(dst));
    EXPECT_FALSE(std::filesystem::exists(src));
}

TEST_F(OperationFixture, UndoCreatesMissingSourceParent) {
    // Muse review: if the source's parent directory was removed
    // after the move, undo must recreate it, not report stale.
    auto sub = dir / "sub";
    std::filesystem::create_directories(sub);
    auto src = sub / "a.txt";
    auto dst = dir / "a-moved.txt";
    { std::ofstream(src) << "abc"; }
    std::filesystem::rename(src, dst);
    repo->log_operation(record(fo::core::OperationType::Rename, src.string(), dst.string()));
    std::filesystem::remove_all(sub);

    auto outcome = repo->undo_last_batch();
    ASSERT_EQ(outcome.undone.size(), 1u);
    EXPECT_TRUE(std::filesystem::exists(src));
}

TEST_F(OperationFixture, GetByIdFindsOldRecords) {
    // Muse review: get_by_id scanned the newest 1000 by
    // timestamp. It must find any record by primary key.
    int64_t first_id = repo->log_operation(record(fo::core::OperationType::Move, "a.txt", "b.txt"));
    for (int i = 0; i < 1005; ++i) {
        repo->log_operation(record(fo::core::OperationType::Move, "x.txt", "y.txt"));
    }
    auto found = repo->get_by_id(first_id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->id, first_id);
}

TEST_F(OperationFixture, UndoRefusesSameSizeContentEdit) {
    // Writers now store the destination's fast64 in file_hash at
    // journal time, so undo verifies CONTENT, not just size: a
    // same-size edit with a backdated mtime after a move must
    // still be refused.
    fo::core::register_all_providers();
    auto hasher = fo::core::Registry<fo::core::IHasher>::instance().create("fast64");
    ASSERT_NE(hasher, nullptr);

    auto src = dir / "orig.txt";
    auto dst = dir / "moved.txt";
    { std::ofstream(src) << "abc"; }
    auto orig_mtime = std::filesystem::last_write_time(src);
    std::filesystem::rename(src, dst);
    auto rec = record(fo::core::OperationType::Rename, src.string(), dst.string());
    rec.file_hash = hasher->fast64(dst);
    ASSERT_FALSE(rec.file_hash.empty());
    repo->log_operation(rec);

    { std::ofstream(dst, std::ios::trunc) << "xyz"; } // same size, new bytes
    std::error_code ec;
    std::filesystem::last_write_time(dst, orig_mtime, ec); // backdated

    auto outcome = repo->undo_last_batch();
    EXPECT_TRUE(outcome.undone.empty());
    ASSERT_EQ(outcome.skipped.size(), 1u);
    EXPECT_NE(outcome.skipped[0].second.find("changed"), std::string::npos);
    EXPECT_TRUE(std::filesystem::exists(dst));
    std::ifstream in(dst);
    std::stringstream ss; ss << in.rdbuf();
    EXPECT_EQ(ss.str(), "xyz"); // user's edit untouched
}

TEST_F(OperationFixture, UndoPrunesOnlyJournaledCreatedDirs) {
    // Round-5 review: pruning by climbing parents can delete a
    // pre-existing empty directory and climb above the organize
    // root. Only directories journaled as created by the batch
    // (log_created_dirs) may be pruned, deepest first.
    auto batch = repo->allocate_batch_id();
    auto created = dir / "cats" / "txt";
    std::filesystem::create_directories(created);
    repo->log_created_dirs(batch, {created.string(), (dir / "cats").string()});
    auto preexisting = dir / "preexisting-empty";
    std::filesystem::create_directories(preexisting);

    auto src = dir / "a.txt";
    auto dst = created / "a.txt";
    { std::ofstream(src) << "abc"; }
    std::filesystem::rename(src, dst);
    auto rec = record(fo::core::OperationType::Move, src.string(), dst.string());
    rec.batch_id = batch;
    repo->log_operation(rec);

    auto outcome = repo->undo_last_batch();
    ASSERT_EQ(outcome.undone.size(), 1u);
    EXPECT_TRUE(std::filesystem::exists(src));
    EXPECT_FALSE(std::filesystem::exists(created));      // journaled: pruned
    EXPECT_FALSE(std::filesystem::exists(dir / "cats")); // journaled: pruned
    EXPECT_TRUE(std::filesystem::exists(preexisting));   // not journaled: survives
    EXPECT_TRUE(std::filesystem::exists(dir));           // root: survives
}

TEST_F(OperationFixture, UndoRefusesMtimeOnlyEditAboveHashCap) {
    // Round-6 review: above the journal-hash cap, identity was
    // size-only. dest_mtime (Migration 11) is recorded for every
    // file, so a same-size in-place edit with a new mtime is
    // refused without reading a byte.
    auto src = dir / "big.bin";
    auto dst = dir / "big-moved.bin";
    { std::ofstream(src) << "abc"; }
    auto orig_mtime = std::filesystem::last_write_time(src);
    std::filesystem::rename(src, dst);
    auto rec = record(fo::core::OperationType::Move, src.string(), dst.string());
    rec.dest_mtime = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::clock_cast<std::chrono::system_clock>(orig_mtime).time_since_epoch()).count();
    repo->log_operation(rec);

    { std::ofstream(dst, std::ios::trunc) << "xyz"; } // same size
    std::error_code ec;
    std::filesystem::last_write_time(dst, orig_mtime + std::chrono::hours(2), ec);

    auto outcome = repo->undo_last_batch();
    EXPECT_TRUE(outcome.undone.empty());
    ASSERT_EQ(outcome.skipped.size(), 1u);
    EXPECT_NE(outcome.skipped[0].second.find("mtime differs"), std::string::npos);
    EXPECT_TRUE(std::filesystem::exists(dst));
}

TEST_F(OperationFixture, UndoKeepsJournaledDirWithUserContent) {
    // Round-6 review: if the user drops their own file into a
    // journaled category dir after organize, the empty-only
    // prune must leave that directory (and their file) alone.
    auto batch = repo->allocate_batch_id();
    auto created = dir / "cats";
    std::filesystem::create_directories(created);
    repo->log_created_dirs(batch, {created.string()});

    auto src = dir / "a.txt";
    auto dst = created / "a.txt";
    { std::ofstream(src) << "abc"; }
    std::filesystem::rename(src, dst);
    auto rec = record(fo::core::OperationType::Move, src.string(), dst.string());
    rec.batch_id = batch;
    repo->log_operation(rec);

    { std::ofstream(created / "user-file.txt") << "mine"; }

    auto outcome = repo->undo_last_batch();
    ASSERT_EQ(outcome.undone.size(), 1u);
    EXPECT_TRUE(std::filesystem::exists(created / "user-file.txt"));
    EXPECT_TRUE(std::filesystem::exists(created));
}

TEST_F(OperationFixture, CopyUndoRefusesCopyEditedAfterCopying) {
    // Muse review: same-size edits slip past the size check,
    // and Copy undo deletes the destination. A copy whose mtime
    // postdates the journal entry must be refused.
    auto dst = dir / "copy.txt";
    { std::ofstream(dst) << "abc"; }
    auto rec = record(fo::core::OperationType::Copy, (dir / "src.txt").string(), dst.string());
    rec.timestamp = std::chrono::system_clock::now() - std::chrono::hours(1);
    repo->log_operation(rec);

    auto outcome = repo->undo_last_batch();
    EXPECT_TRUE(outcome.undone.empty());
    ASSERT_EQ(outcome.skipped.size(), 1u);
    EXPECT_NE(outcome.skipped[0].second.find("modified after"), std::string::npos);
    EXPECT_TRUE(std::filesystem::exists(dst));
}

TEST_F(OperationFixture, BatchIdsAreUniqueAcrossInterleavedAllocations) {
    // Muse review: allocate_batch_id was MAX+1 with no
    // transaction, so two invocations allocating before either
    // logged a row received the SAME id and their records merged
    // into one batch. Allocation must be atomic.
    fo::core::OperationRepository other(*db);
    auto a = repo->allocate_batch_id();
    auto b = other.allocate_batch_id();
    EXPECT_NE(a, b);
    auto c = repo->allocate_batch_id();
    EXPECT_NE(c, a);
    EXPECT_NE(c, b);
}

TEST_F(OperationFixture, UndoWithEmptyJournalReturnsNullopt) {
    EXPECT_FALSE(repo->undo_last().has_value());
}

} // namespace
