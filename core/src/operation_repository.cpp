#include "fo/core/operation_repository.hpp"
#include "fo/core/fault_injection.hpp"
#include "fo/core/interfaces.hpp"
#include "fo/core/registry.hpp"
#include <sqlite3.h>
#include <algorithm>
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/stat.h>
#endif
#include <stdexcept>
#include <filesystem>

namespace fo::core {

OperationRepository::OperationRepository(DatabaseManager& db) : db_(db) {}

std::string OperationRepository::file_identity(const std::filesystem::path& path) {
#ifdef _WIN32
    HANDLE h = CreateFileW(path.c_str(), FILE_READ_ATTRIBUTES,
                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                           nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return "";
    BY_HANDLE_FILE_INFORMATION info{};
    std::string out;
    if (GetFileInformationByHandle(h, &info)) {
        uint64_t index = (static_cast<uint64_t>(info.nFileIndexHigh) << 32) | info.nFileIndexLow;
        out = std::to_string(info.dwVolumeSerialNumber) + ":" + std::to_string(index);
    }
    CloseHandle(h);
    return out;
#else
    struct stat st {};
    if (::stat(path.c_str(), &st) != 0) return "";
    return std::to_string(static_cast<uint64_t>(st.st_dev)) + ":" +
           std::to_string(static_cast<uint64_t>(st.st_ino));
#endif
}

std::string OperationRepository::operation_type_to_string(OperationType type) {
    switch (type) {
        case OperationType::Move: return "move";
        case OperationType::Copy: return "copy";
        case OperationType::Delete: return "delete";
        case OperationType::Rename: return "rename";
        default: return "unknown";
    }
}

OperationType OperationRepository::string_to_operation_type(const std::string& str) {
    if (str == "move") return OperationType::Move;
    if (str == "copy") return OperationType::Copy;
    if (str == "delete") return OperationType::Delete;
    if (str == "rename") return OperationType::Rename;
    return OperationType::Move; // default
}

int64_t OperationRepository::allocate_batch_id() {
    sqlite3* db = db_.get_db();
    // Atomic increment inside a write transaction (Migration 9's
    // counter table): the old MAX+1 read let two concurrent
    // invocations receive the same id before either logged a row,
    // silently merging two commands into one undo batch.
    sqlite3_exec(db, "BEGIN IMMEDIATE;", nullptr, nullptr, nullptr);
    sqlite3_exec(db,
        "INSERT INTO batch_counter (id, value) VALUES (1, 1) "
        "ON CONFLICT(id) DO UPDATE SET value = value + 1;",
        nullptr, nullptr, nullptr);
    int64_t next = 0;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, "SELECT value FROM batch_counter WHERE id = 1;", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) next = sqlite3_column_int64(stmt, 0);
        sqlite3_finalize(stmt);
    }
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    if (next > 0) return next;
    // Counter unavailable (unmigrated database): legacy fallback.
    if (sqlite3_prepare_v2(db, "SELECT COALESCE(MAX(batch_id), 0) + 1 FROM operation_log;", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) next = sqlite3_column_int64(stmt, 0);
        sqlite3_finalize(stmt);
    }
    return next > 0 ? next : 1;
}

int64_t OperationRepository::log_operation(const OperationRecord& record) {
    sqlite3* db = db_.get_db();

    const char* sql = R"(
        INSERT INTO operation_log (timestamp, operation_type, source_path, dest_path, file_size, file_hash, status, undone, batch_id, dest_mtime, dest_file_id)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    auto ts = std::chrono::duration_cast<std::chrono::seconds>(record.timestamp.time_since_epoch()).count();

    std::string op_type = operation_type_to_string(record.type);
    std::string src = record.source_path;
    std::string dst = record.dest_path;
    std::string fh = record.file_hash;
    std::string st = record.status;

    sqlite3_bind_int64(stmt, 1, ts);
    sqlite3_bind_text(stmt, 2, op_type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, src.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, dst.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 5, record.file_size);
    sqlite3_bind_text(stmt, 6, fh.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, st.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 8, record.undone ? 1 : 0);
    // Unbatched writes (legacy callers, single ops) each become
    // their own batch so undo_last_batch treats them singly.
    sqlite3_bind_int64(stmt, 9, record.batch_id > 0 ? record.batch_id : allocate_batch_id());
    sqlite3_bind_int64(stmt, 10, record.dest_mtime);
    sqlite3_bind_text(stmt, 11, record.dest_file_id.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to insert operation: " + std::string(sqlite3_errmsg(db)));
    }

    int64_t id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    return id;
}

std::vector<OperationRecord> OperationRepository::get_all(int limit) {
    sqlite3* db = db_.get_db();
    std::vector<OperationRecord> results;

    std::string sql = "SELECT id, timestamp, operation_type, source_path, dest_path, file_size, file_hash, status, undone, batch_id, dest_mtime, dest_file_id "
                      "FROM operation_log ORDER BY timestamp DESC LIMIT " + std::to_string(limit);

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        OperationRecord rec;
        rec.id = sqlite3_column_int64(stmt, 0);
        rec.timestamp = std::chrono::system_clock::time_point(std::chrono::seconds(sqlite3_column_int64(stmt, 1)));
        rec.type = string_to_operation_type(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        rec.source_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        if (sqlite3_column_text(stmt, 4)) rec.dest_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        rec.file_size = sqlite3_column_int64(stmt, 5);
        if (sqlite3_column_text(stmt, 6)) rec.file_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        if (sqlite3_column_text(stmt, 7)) rec.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        rec.undone = sqlite3_column_int(stmt, 8) != 0;
        rec.batch_id = sqlite3_column_int64(stmt, 9);
        rec.dest_mtime = sqlite3_column_int64(stmt, 10);
        if (sqlite3_column_text(stmt, 11)) rec.dest_file_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        results.push_back(rec);
    }

    sqlite3_finalize(stmt);
    return results;
}

std::vector<OperationRecord> OperationRepository::get_undoable(int limit) {
    sqlite3* db = db_.get_db();
    std::vector<OperationRecord> results;

    // Only reversible operations are undoable: a journaled Delete
    // cannot be reversed (no backup), and returning it here made
    // undo_last pick it, fail silently, and block undoing every
    // earlier Move/Rename forever.
    // id order, not timestamp: timestamps are second-resolution, so
    // same-second records of different batches could interleave and
    // break undo_last_batch's contiguous grouping.
    std::string sql = "SELECT id, timestamp, operation_type, source_path, dest_path, file_size, file_hash, status, undone, batch_id, dest_mtime, dest_file_id "
                      "FROM operation_log WHERE undone = 0 AND status = 'completed' AND operation_type != 'delete' ORDER BY id DESC LIMIT " + std::to_string(limit);

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        OperationRecord rec;
        rec.id = sqlite3_column_int64(stmt, 0);
        rec.timestamp = std::chrono::system_clock::time_point(std::chrono::seconds(sqlite3_column_int64(stmt, 1)));
        rec.type = string_to_operation_type(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        rec.source_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        if (sqlite3_column_text(stmt, 4)) rec.dest_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        rec.file_size = sqlite3_column_int64(stmt, 5);
        if (sqlite3_column_text(stmt, 6)) rec.file_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        if (sqlite3_column_text(stmt, 7)) rec.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        rec.undone = sqlite3_column_int(stmt, 8) != 0;
        rec.batch_id = sqlite3_column_int64(stmt, 9);
        rec.dest_mtime = sqlite3_column_int64(stmt, 10);
        if (sqlite3_column_text(stmt, 11)) rec.dest_file_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        results.push_back(rec);
    }

    sqlite3_finalize(stmt);
    return results;
}

void OperationRepository::mark_undone(int64_t id) {
    db_.execute("UPDATE operation_log SET undone = 1, status = 'completed' WHERE id = " +
                std::to_string(id));
}

void OperationRepository::set_status(int64_t id, const std::string& status) {
    sqlite3* db = db_.get_db();
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, "UPDATE operation_log SET status = ? WHERE id = ?;",
                           -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void OperationRepository::delete_operation(int64_t id) {
    db_.execute("DELETE FROM operation_log WHERE id = " + std::to_string(id));
}

void OperationRepository::complete_operation(int64_t id, const OperationRecord& identity) {
    sqlite3* db = db_.get_db();
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db,
            "UPDATE operation_log SET status = 'completed', file_hash = ?, "
            "dest_mtime = ?, dest_file_id = ? WHERE id = ?;",
            -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, identity.file_hash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 2, identity.dest_mtime);
    sqlite3_bind_text(stmt, 3, identity.dest_file_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 4, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

// reconcile_pending is implemented after read_record/kRecordCols
// below (it reuses them); see the definition past undo_last_batch.

namespace {
OperationRecord read_record(sqlite3_stmt* stmt) {
    OperationRecord rec;
    rec.id = sqlite3_column_int64(stmt, 0);
    rec.timestamp = std::chrono::system_clock::time_point(std::chrono::seconds(sqlite3_column_int64(stmt, 1)));
    rec.type = OperationRepository::string_to_operation_type(
        reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
    rec.source_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
    if (sqlite3_column_text(stmt, 4)) rec.dest_path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    rec.file_size = sqlite3_column_int64(stmt, 5);
    if (sqlite3_column_text(stmt, 6)) rec.file_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
    if (sqlite3_column_text(stmt, 7)) rec.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
    rec.undone = sqlite3_column_int(stmt, 8) != 0;
    rec.batch_id = sqlite3_column_int64(stmt, 9);
    rec.dest_mtime = sqlite3_column_int64(stmt, 10);
    if (sqlite3_column_text(stmt, 11)) rec.dest_file_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
    return rec;
}

const char* kRecordCols =
    "id, timestamp, operation_type, source_path, dest_path, file_size, file_hash, status, undone, batch_id, dest_mtime, dest_file_id";
} // namespace

std::optional<OperationRecord> OperationRepository::get_by_id(int64_t id) {
    // Primary-key lookup: the old scan of the newest 1000 rows by
    // timestamp silently returned nullopt for older records.
    sqlite3* db = db_.get_db();
    std::string sql = std::string("SELECT ") + kRecordCols + " FROM operation_log WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_int64(stmt, 1, id);
    std::optional<OperationRecord> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) result = read_record(stmt);
    sqlite3_finalize(stmt);
    return result;
}

void OperationRepository::clear_old(int days) {
    auto cutoff = std::chrono::system_clock::now() - std::chrono::hours(24 * days);
    auto ts = std::chrono::duration_cast<std::chrono::seconds>(cutoff.time_since_epoch()).count();
    db_.execute("DELETE FROM operation_log WHERE timestamp < " + std::to_string(ts));
}

namespace {
// fast64 of a file via the registered hasher, or "" when no
// hasher is registered in this process (verification then falls
// back to size/mtime only, as pre-hash journals require).
std::string content_fast64(const std::filesystem::path& p) {
    static auto hasher = Registry<IHasher>::instance().create("fast64");
    if (!hasher) return "";
    try { return hasher->fast64(p); } catch (...) { return ""; }
}

// Shared identity gate for Move/Rename/Copy: size first, then —
// when the journal carries a content hash (writers record the
// destination's fast64 since v6.5.75) — content. Either mismatch
// means the file was modified after the operation.
std::optional<std::string> verify_identity(const OperationRecord& rec,
                                           const std::filesystem::path& dest) {
    std::error_code ec;
    if (rec.file_size > 0 && std::filesystem::is_regular_file(dest, ec) &&
        std::filesystem::file_size(dest, ec) != static_cast<std::uintmax_t>(rec.file_size)) {
        return "file changed since the operation (size differs)";
    }
    if (!rec.file_hash.empty() && std::filesystem::is_regular_file(dest, ec)) {
        auto now = content_fast64(dest);
        if (!now.empty() && now != rec.file_hash) {
            return "file changed since the operation (content differs)";
        }
    }
    // File identity (Migration 12): survives renames, not
    // replacement — catches a different file wearing the same
    // path with forged size and mtime.
    bool same_file_id = false;
    if (!rec.dest_file_id.empty() && std::filesystem::is_regular_file(dest, ec)) {
        auto now_id = OperationRepository::file_identity(dest);
        if (!now_id.empty() && now_id != rec.dest_file_id) {
            return "file changed since the operation (identity differs)";
        }
        same_file_id = !now_id.empty();
    }
    // Mtime identity (Migration 11): recorded for every file,
    // including ones too large for journal-time hashing, so a
    // same-size in-place edit is caught without reading a byte.
    // FAT/exFAT round mtimes to 2s, so when the file ID proves
    // this is the same file, <=2s of drift is filesystem
    // rounding, not an edit (an hour-scale DST shift on FAT
    // still refuses — the safe direction). A replacement
    // still fails the file-ID gate above; an in-place edit of a
    // hashable file still fails the content gate.
    if (rec.dest_mtime > 0 && std::filesystem::is_regular_file(dest, ec)) {
        std::error_code mec;
        auto mt = std::filesystem::last_write_time(dest, mec);
        if (!mec) {
            auto secs = std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::clock_cast<std::chrono::system_clock>(mt).time_since_epoch()).count();
            auto drift = secs - rec.dest_mtime;
            if (drift < 0) drift = -drift;
            if (secs != rec.dest_mtime && !(same_file_id && drift <= 2)) {
                return "file changed since the operation (mtime differs)";
            }
        }
    }
    return std::nullopt;
}

// Attempt to reverse one journal record. Returns nullopt on
// success, otherwise a human-readable refusal reason. Identity is
// verified before touching anything: a destination whose size no
// longer matches the journal was modified after the operation
// and must never be moved/deleted by undo.
std::optional<std::string> try_undo_record(const OperationRecord& rec) {
    std::error_code ec;
    auto dest = std::filesystem::path(rec.dest_path);
    auto src = std::filesystem::path(rec.source_path);
    try {
        switch (rec.type) {
            case OperationType::Move:
            case OperationType::Rename: {
                if (!std::filesystem::exists(dest, ec)) return "destination no longer exists";
                if (std::filesystem::exists(src, ec)) return "source path was recreated";
                if (auto bad = verify_identity(rec, dest)) return bad;
                // The source's parent may have been removed after
                // the move; recreate it instead of failing stale.
                if (src.has_parent_path()) {
                    std::filesystem::create_directories(src.parent_path(), ec);
                }
                std::filesystem::rename(dest, src);
                fault_checkpoint("undo.after-rename");
                return std::nullopt;
            }
            case OperationType::Copy: {
                if (!std::filesystem::exists(dest, ec)) return "destination no longer exists";
                if (auto bad = verify_identity(rec, dest)) return bad;
                // Same-size edits are invisible to the size check,
                // and Copy undo DELETES the destination: refuse
                // when the copy's mtime postdates the journal
                // entry (rename-back preserves mtime, so this
                // guard is Copy-only).
                std::error_code mec;
                auto mt = std::filesystem::last_write_time(dest, mec);
                if (!mec) {
                    auto mt_sys = std::chrono::clock_cast<std::chrono::system_clock>(mt);
                    if (mt_sys > rec.timestamp + std::chrono::seconds(2)) {
                        return "file changed since the operation (modified after it was copied)";
                    }
                }
                std::filesystem::remove(dest);
                fault_checkpoint("undo.after-rename");
                return std::nullopt;
            }
            case OperationType::Delete:
                return "deletions cannot be undone";
        }
    } catch (const std::exception& e) {
        return std::string("I/O error: ") + e.what();
    }
    return "unknown operation type";
}

// Newest non-undone, completed record of any type (deletes
// included), or nullopt. Undo must see delete batches to refuse
// them instead of silently reversing an older batch.
std::optional<OperationRecord> newest_open_record(DatabaseManager& db) {
    sqlite3* raw = db.get_db();
    std::string sql = std::string("SELECT ") + kRecordCols +
        " FROM operation_log WHERE undone = 0 AND status = 'completed' ORDER BY id DESC LIMIT 1;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    std::optional<OperationRecord> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) result = read_record(stmt);
    sqlite3_finalize(stmt);
    return result;
}

// All open records of one batch (batch_id 0 = the single legacy
// record itself), newest first, with no count cap.
std::vector<OperationRecord> open_batch_members(DatabaseManager& db, const OperationRecord& anchor) {
    sqlite3* raw = db.get_db();
    std::string sql = std::string("SELECT ") + kRecordCols + " FROM operation_log WHERE undone = 0 "
        "AND status = 'completed' AND " +
        (anchor.batch_id != 0 ? "batch_id = ?" : "id = ?") + " ORDER BY id DESC;";
    sqlite3_stmt* stmt;
    std::vector<OperationRecord> results;
    if (sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return results;
    sqlite3_bind_int64(stmt, 1, anchor.batch_id != 0 ? anchor.batch_id : anchor.id);
    while (sqlite3_step(stmt) == SQLITE_ROW) results.push_back(read_record(stmt));
    sqlite3_finalize(stmt);
    return results;
}
} // namespace

namespace {
void prune_created_dirs(DatabaseManager& db, int64_t batch_id);
} // namespace

std::optional<OperationRecord> OperationRepository::undo_last() {
    // Walk recent records newest-first and undo the first one that is
    // still reversible. A record whose destination vanished externally
    // (or whose move-back fails) is stale: skipping it keeps one bad
    // record from blocking every older undo forever.
    for (const auto& rec : get_undoable(50)) {
        set_status(rec.id, "undoing");
        if (!try_undo_record(rec)) {
            mark_undone(rec.id);
            return rec;
        }
        set_status(rec.id, "completed");
    }

    return std::nullopt;
}

OperationRepository::UndoOutcome OperationRepository::undo_last_batch() {
    // One uncapped walk over every open record (deletes included),
    // grouped into batches newest-first. The old get_undoable(500)
    // silently truncated bigger batches mid-undo.
    UndoOutcome outcome;
    sqlite3* raw = db_.get_db();
    std::string sql = std::string("SELECT ") + kRecordCols +
        " FROM operation_log WHERE undone = 0 AND status = 'completed' ORDER BY id DESC;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return outcome;
    std::vector<OperationRecord> open;
    while (sqlite3_step(stmt) == SQLITE_ROW) open.push_back(read_record(stmt));
    sqlite3_finalize(stmt);

    size_t i = 0;
    bool first_batch = true;
    while (i < open.size()) {
        // Gather one batch: same batch_id, or a legacy singleton.
        const int64_t batch = open[i].batch_id;
        size_t j = i;
        std::vector<OperationRecord> members;
        while (j < open.size()) {
            bool same = (batch != 0) ? (open[j].batch_id == batch) : true;
            if (!same) break;
            members.push_back(open[j]);
            ++j;
            if (batch == 0) break;
        }

        // A batch containing deletions is irreversible. Refuse the
        // NEWEST one loudly; never silently reverse an older batch
        // instead (dismiss_newest_batch is the explicit way past).
        int deletes = 0;
        for (const auto& m : members) if (m.type == OperationType::Delete) ++deletes;
        if (deletes > 0 && first_batch) {
            outcome.refused_delete_batch = true;
            outcome.delete_batch_records = deletes;
            return outcome;
        }

        size_t undone_before = outcome.undone.size();
        for (const auto& rec : members) {
            if (rec.type == OperationType::Delete) continue; // older delete batches: skip silently in the walk
            set_status(rec.id, "undoing");
            if (auto reason = try_undo_record(rec)) {
                set_status(rec.id, "completed");
                outcome.skipped.push_back({rec, *reason});
            } else {
                mark_undone(rec.id);
                outcome.undone.push_back(rec);
            }
        }
        if (outcome.undone.size() > undone_before) {
            prune_created_dirs(db_, batch);
            return outcome;
        }
        ++outcome.skipped_batches; // wholly stale batch: report + fall through
        first_batch = false;
        i = j;
    }
    return outcome;
}

int OperationRepository::reconcile_pending() {
    sqlite3* raw = db_.get_db();
    // Hold the write lock for the whole sweep so a concurrent
    // invocation cannot reconcile the same rows twice.
    sqlite3_exec(raw, "BEGIN IMMEDIATE;", nullptr, nullptr, nullptr);
    std::vector<OperationRecord> open;
    sqlite3_stmt* stmt;
    std::string sql = std::string("SELECT ") + kRecordCols +
        " FROM operation_log WHERE undone = 0 AND status IN ('pending', 'undoing');";
    if (sqlite3_prepare_v2(raw, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) open.push_back(read_record(stmt));
        sqlite3_finalize(stmt);
    }
    int resolved = 0;
    std::error_code ec;
    for (const auto& rec : open) {
        auto src = std::filesystem::path(rec.source_path);
        auto dst = std::filesystem::path(rec.dest_path);
        bool at_src = !src.empty() && std::filesystem::exists(src, ec);
        bool at_dst = !dst.empty() && std::filesystem::exists(dst, ec);
        if (rec.status == "pending") {
            // Intent row written before the rename. Rename never
            // ran (file home): drop the intent. Rename landed
            // (file at dest only): the journal becomes truth by
            // completing the row — undo can now restore it.
            if (at_src && !at_dst) { delete_operation(rec.id); ++resolved; }
            else if (at_dst && !at_src) { set_status(rec.id, "completed"); ++resolved; }
        } else { // "undoing": undo's rename may or may not have run
            bool restored = false;
            if (rec.type == OperationType::Copy) {
                restored = !at_dst; // copy deleted by undo
            } else {
                restored = at_src && !at_dst; // moved back home
            }
            if (restored) { mark_undone(rec.id); ++resolved; }
            else if (at_dst) { set_status(rec.id, "completed"); ++resolved; }
        }
        // Both paths exist (or neither) is ambiguous: leave the
        // row for a human rather than guess at user files.
    }
    sqlite3_exec(raw, "COMMIT;", nullptr, nullptr, nullptr);
    return resolved;
}

void OperationRepository::log_created_dirs(int64_t batch_id, const std::vector<std::string>& dirs) {
    sqlite3* db = db_.get_db();
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, "INSERT INTO created_dirs (batch_id, path) VALUES (?, ?);", -1, &stmt, nullptr) != SQLITE_OK) return;
    for (const auto& d : dirs) {
        sqlite3_bind_int64(stmt, 1, batch_id);
        sqlite3_bind_text(stmt, 2, d.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }
    sqlite3_finalize(stmt);
}

namespace {
// Prune one batch's journaled created dirs, deepest first. Only
// empty directories are removed (remove() fails otherwise), and
// only journaled ones — a pre-existing directory the batch
// merely emptied is user state, never undo's to delete.
void prune_created_dirs(DatabaseManager& db, int64_t batch_id) {
    if (batch_id == 0) return;
    sqlite3* raw = db.get_db();
    sqlite3_stmt* stmt;
    std::vector<std::string> dirs;
    if (sqlite3_prepare_v2(raw, "SELECT path FROM created_dirs WHERE batch_id = ?;", -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, batch_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            if (auto* t = sqlite3_column_text(stmt, 0)) dirs.emplace_back(reinterpret_cast<const char*>(t));
        }
        sqlite3_finalize(stmt);
    }
    auto depth = [](const std::string& s) {
        return std::count_if(s.begin(), s.end(), [](char c) { return c == '/' || c == '\\'; });
    };
    std::sort(dirs.begin(), dirs.end(), [&](const std::string& a, const std::string& b) {
        return depth(a) > depth(b);
    });
    for (const auto& d : dirs) {
        std::error_code ec;
        std::filesystem::remove(std::filesystem::path(d), ec); // empty only
    }
    db.execute("DELETE FROM created_dirs WHERE batch_id = " + std::to_string(batch_id) + ";");
}
} // namespace

int OperationRepository::dismiss_newest_batch() {
    auto anchor = newest_open_record(db_);
    if (!anchor) return 0;
    auto members = open_batch_members(db_, *anchor);
    for (const auto& m : members) mark_undone(m.id);
    return static_cast<int>(members.size());
}

} // namespace fo::core

