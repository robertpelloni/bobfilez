#pragma once

#include "database.hpp"
#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <optional>

namespace fo::core {

enum class OperationType {
    Move,
    Copy,
    Delete,
    Rename
};

struct OperationRecord {
    int64_t id = 0;
    std::chrono::system_clock::time_point timestamp;
    OperationType type;
    std::string source_path;
    std::string dest_path;
    int64_t file_size = 0;
    std::string file_hash;
    std::string status = "completed";
    bool undone = false;
    // Command batch this record belongs to (Migration 8). Records
    // from one multi-file command share a batch; 0 = legacy row,
    // treated as its own batch.
    int64_t batch_id = 0;
    // Destination mtime (unix seconds) at journal time
    // (Migration 11); 0 = not recorded. Undo compares it for
    // every file, including ones too large to hash.
    int64_t dest_mtime = 0;
    // Destination file identity at journal time (Migration 12):
    // "volume:index" (NTFS) or "dev:inode" (POSIX); empty = not
    // recorded. Last field on purpose: aggregate initializers
    // elsewhere must not shift.
    std::string dest_file_id;
};

class OperationRepository {
public:
    explicit OperationRepository(DatabaseManager& db);

    // Log a new operation
    int64_t log_operation(const OperationRecord& record);

    // Get all operations (most recent first)
    std::vector<OperationRecord> get_all(int limit = 100);

    // Get operations that can be undone
    std::vector<OperationRecord> get_undoable(int limit = 100);

    // Mark an operation as undone
    void mark_undone(int64_t id);

    // Crash-recovery plumbing. Mutating commands journal an
    // intent row (status "pending") BEFORE touching the
    // filesystem, then complete it after the rename lands;
    // undo marks records "undoing" before its rename and
    // restores "completed" when the rename is journaled.
    // set_status/delete_operation drive those transitions, and
    // reconcile_pending resolves rows a crash left mid-flight
    // by comparing journal paths against the live tree.
    void set_status(int64_t id, const std::string& status);
    void delete_operation(int64_t id);
    // Fill in post-rename identity fields and mark completed.
    void complete_operation(int64_t id, const OperationRecord& identity);
    // Resolve every pending/undoing row against the filesystem.
    // Returns how many rows were resolved; ambiguous rows (both
    // paths exist, or neither) stay untouched for a human.
    int reconcile_pending();

    // Allocate a fresh batch id for one multi-file command.
    int64_t allocate_batch_id();

    // Record directories a batch created (organize target
    // parents). Batch undo prunes exactly these (deepest first,
    // only when empty); pre-existing dirs are never pruned.
    void log_created_dirs(int64_t batch_id, const std::vector<std::string>& dirs);

    // Undo the most recent undoable operation
    std::optional<OperationRecord> undo_last();

    // Outcome of one batch-undo attempt. `skipped` carries a
    // human-readable reason per record that could not be reversed
    // (callers must surface these; silent partial undo is how
    // users lose track of files). `refused_delete_batch` is set
    // when the newest batch is a deletion batch: deletions are
    // irreversible, and quietly reversing an *older* batch
    // instead would be a lie — callers must refuse loudly and
    // offer dismiss_newest_batch() as the explicit way past it.
    struct UndoOutcome {
        std::vector<OperationRecord> undone;
        std::vector<std::pair<OperationRecord, std::string>> skipped;
        int skipped_batches = 0;
        bool refused_delete_batch = false;
        int delete_batch_records = 0;
    };

    // Undo every undoable record in the most recent batch,
    // newest first, with no record-count cap. Stale records are
    // reported in `skipped`; a batch that yields nothing falls
    // through to the next older batch, mirroring undo_last.
    UndoOutcome undo_last_batch();

    // Mark the newest non-undone batch as acknowledged without
    // reversing it (the explicit escape after a delete batch).
    // Returns the number of records dismissed.
    int dismiss_newest_batch();

    // Get operation by ID
    std::optional<OperationRecord> get_by_id(int64_t id);

    // Clear old operations (older than days)
    void clear_old(int days = 30);

    static OperationType string_to_operation_type(const std::string& str);

    // Platform file identity that survives renames but not
    // replacement: "volume:index" on Windows, "dev:inode" on
    // POSIX, "" when it cannot be determined.
    static std::string file_identity(const std::filesystem::path& path);

private:
    DatabaseManager& db_;

    static std::string operation_type_to_string(OperationType type);
};

} // namespace fo::core

