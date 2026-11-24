#include "fo/core/operation_repository.hpp"
#include <sqlite3.h>
#include <stdexcept>
#include <filesystem>

namespace fo::core {

OperationRepository::OperationRepository(DatabaseManager& db) : db_(db) {}

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
        INSERT INTO operation_log (timestamp, operation_type, source_path, dest_path, file_size, file_hash, status, undone, batch_id)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
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

    std::string sql = "SELECT id, timestamp, operation_type, source_path, dest_path, file_size, file_hash, status, undone, batch_id "
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
    std::string sql = "SELECT id, timestamp, operation_type, source_path, dest_path, file_size, file_hash, status, undone, batch_id "
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
        results.push_back(rec);
    }

    sqlite3_finalize(stmt);
    return results;
}

void OperationRepository::mark_undone(int64_t id) {
    db_.execute("UPDATE operation_log SET undone = 1 WHERE id = " + std::to_string(id));
}

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
    return rec;
}

const char* kRecordCols =
    "id, timestamp, operation_type, source_path, dest_path, file_size, file_hash, status, undone, batch_id";
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
                if (rec.file_size > 0 && std::filesystem::is_regular_file(dest, ec) &&
                    std::filesystem::file_size(dest, ec) != static_cast<std::uintmax_t>(rec.file_size)) {
                    return "file changed since the operation (size differs)";
                }
                // The source's parent may have been removed after
                // the move; recreate it instead of failing stale.
                if (src.has_parent_path()) {
                    std::filesystem::create_directories(src.parent_path(), ec);
                }
                std::filesystem::rename(dest, src);
                return std::nullopt;
            }
            case OperationType::Copy: {
                if (!std::filesystem::exists(dest, ec)) return "destination no longer exists";
                if (rec.file_size > 0 && std::filesystem::is_regular_file(dest, ec) &&
                    std::filesystem::file_size(dest, ec) != static_cast<std::uintmax_t>(rec.file_size)) {
                    return "file changed since the operation (size differs)";
                }
                std::filesystem::remove(dest);
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

std::optional<OperationRecord> OperationRepository::undo_last() {
    // Walk recent records newest-first and undo the first one that is
    // still reversible. A record whose destination vanished externally
    // (or whose move-back fails) is stale: skipping it keeps one bad
    // record from blocking every older undo forever.
    for (const auto& rec : get_undoable(50)) {
        if (!try_undo_record(rec)) {
            mark_undone(rec.id);
            return rec;
        }
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
            if (auto reason = try_undo_record(rec)) {
                outcome.skipped.push_back({rec, *reason});
            } else {
                mark_undone(rec.id);
                outcome.undone.push_back(rec);
            }
        }
        if (outcome.undone.size() > undone_before) return outcome;
        ++outcome.skipped_batches; // wholly stale batch: report + fall through
        first_batch = false;
        i = j;
    }
    return outcome;
}

int OperationRepository::dismiss_newest_batch() {
    auto anchor = newest_open_record(db_);
    if (!anchor) return 0;
    auto members = open_batch_members(db_, *anchor);
    for (const auto& m : members) mark_undone(m.id);
    return static_cast<int>(members.size());
}

} // namespace fo::core

