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
    sqlite3_stmt* stmt;
    int64_t next = 1;
    if (sqlite3_prepare_v2(db, "SELECT COALESCE(MAX(batch_id), 0) + 1 FROM operation_log;", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) next = sqlite3_column_int64(stmt, 0);
        sqlite3_finalize(stmt);
    }
    return next;
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

std::optional<OperationRecord> OperationRepository::get_by_id(int64_t id) {
    auto all = get_all(1000);
    for (const auto& rec : all) {
        if (rec.id == id) return rec;
    }
    return std::nullopt;
}

void OperationRepository::clear_old(int days) {
    auto cutoff = std::chrono::system_clock::now() - std::chrono::hours(24 * days);
    auto ts = std::chrono::duration_cast<std::chrono::seconds>(cutoff.time_since_epoch()).count();
    db_.execute("DELETE FROM operation_log WHERE timestamp < " + std::to_string(ts));
}

namespace {
// Attempt to reverse one journal record. Returns false for stale
// records (destination vanished, or the source path was recreated
// externally — MSVC rename would replace and destroy the new file).
bool try_undo_record(const OperationRecord& rec) {
    try {
        switch (rec.type) {
            case OperationType::Move:
            case OperationType::Rename:
                if (std::filesystem::exists(rec.dest_path) &&
                    !std::filesystem::exists(rec.source_path)) {
                    std::filesystem::rename(rec.dest_path, rec.source_path);
                    return true;
                }
                return false;
            case OperationType::Copy:
                if (std::filesystem::exists(rec.dest_path)) {
                    std::filesystem::remove(rec.dest_path);
                    return true;
                }
                return false;
            case OperationType::Delete:
                // Cannot undo delete without backup (filtered out
                // by get_undoable; kept for exhaustiveness).
                return false;
        }
    } catch (const std::exception&) {
        return false; // stale record
    }
    return false;
}
} // namespace

std::optional<OperationRecord> OperationRepository::undo_last() {
    // Walk recent records newest-first and undo the first one that is
    // still reversible. A record whose destination vanished externally
    // (or whose move-back fails) is stale: skipping it keeps one bad
    // record from blocking every older undo forever.
    for (const auto& rec : get_undoable(50)) {
        if (try_undo_record(rec)) {
            mark_undone(rec.id);
            return rec;
        }
    }

    return std::nullopt;
}

std::vector<OperationRecord> OperationRepository::undo_last_batch() {
    // Group undoable records by batch, newest batch first (records
    // arrive newest-first). Legacy batch_id 0 rows each form their
    // own single-record batch, keyed by record id to keep them
    // distinct. A batch whose members are all stale falls through
    // to the next older batch, mirroring undo_last's walk.
    auto undoable = get_undoable(500);
    std::vector<OperationRecord> undone;
    size_t i = 0;
    while (i < undoable.size() && undone.empty()) {
        const int64_t batch = undoable[i].batch_id;
        size_t j = i;
        std::vector<OperationRecord> members;
        while (j < undoable.size()) {
            bool same = (batch != 0) ? (undoable[j].batch_id == batch)
                                     : (undoable[j].batch_id == 0);
            if (!same) break;
            members.push_back(undoable[j]);
            ++j;
            if (batch == 0) break; // legacy rows are singletons
        }
        for (const auto& rec : members) {
            if (try_undo_record(rec)) {
                mark_undone(rec.id);
                undone.push_back(rec);
            }
        }
        i = j;
    }
    return undone;
}

} // namespace fo::core

