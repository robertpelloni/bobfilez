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

int64_t OperationRepository::log_operation(const OperationRecord& record) {
    sqlite3* db = db_.get_db();

    const char* sql = R"(
        INSERT INTO operation_log (timestamp, operation_type, source_path, dest_path, file_size, file_hash, status, undone)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
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

    std::string sql = "SELECT id, timestamp, operation_type, source_path, dest_path, file_size, file_hash, status, undone "
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
        results.push_back(rec);
    }

    sqlite3_finalize(stmt);
    return results;
}

std::vector<OperationRecord> OperationRepository::get_undoable(int limit) {
    sqlite3* db = db_.get_db();
    std::vector<OperationRecord> results;

    std::string sql = "SELECT id, timestamp, operation_type, source_path, dest_path, file_size, file_hash, status, undone "
                      "FROM operation_log WHERE undone = 0 AND status = 'completed' ORDER BY timestamp DESC LIMIT " + std::to_string(limit);

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

std::optional<OperationRecord> OperationRepository::undo_last() {
    auto undoable = get_undoable(1);
    if (undoable.empty()) {
        return std::nullopt;
    }

    const auto& rec = undoable[0];
    bool success = false;

    try {
        switch (rec.type) {
            case OperationType::Move:
            case OperationType::Rename:
                // Move the file back
                if (std::filesystem::exists(rec.dest_path)) {
                    std::filesystem::rename(rec.dest_path, rec.source_path);
                    success = true;
                }
                break;
            case OperationType::Copy:
                // Delete the copy
                if (std::filesystem::exists(rec.dest_path)) {
                    std::filesystem::remove(rec.dest_path);
                    success = true;
                }
                break;
            case OperationType::Delete:
                // Cannot undo delete without backup
                // In a full implementation, we'd restore from a trash/backup location
                break;
        }
    } catch (const std::exception&) {
        // Undo failed
        return std::nullopt;
    }

    if (success) {
        mark_undone(rec.id);
        return rec;
    }

    return std::nullopt;
}

} // namespace fo::core

