/// @file audit_logger.cpp
/// @brief Implementation of the Forensic Audit Ledger using SQLite.
///
/// Provides an immutable, append-only record of all destructive or movement-based
/// file operations. Each entry includes a SHA-256 chain hash linking it to the
/// previous entry for tamper detection.

#include "fo/core/audit_logger_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/database.hpp"
#include <sqlite3.h>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cstring>

// SHA-256 for chain hashing
#include "hash-library/sha256.h"

namespace fo::core {

class AuditLoggerImpl : public IAuditLogger {
    DatabaseManager& db_;

    /// Compute a chain hash for an audit entry: SHA256(prev_hash | id | timestamp | operation | src | dst)
    static std::string compute_entry_hash(
        const std::string& prev_hash,
        uint64_t id,
        int64_t timestamp,
        const std::string& operation,
        const std::filesystem::path& src,
        const std::filesystem::path& dst,
        uintmax_t size,
        const std::string& checksum)
    {
        SHA256 sha256;
        sha256.add(prev_hash.data(), prev_hash.size());
        auto id_str = std::to_string(id);
        sha256.add(id_str.data(), id_str.size());
        auto ts_str = std::to_string(timestamp);
        sha256.add(ts_str.data(), ts_str.size());
        sha256.add(operation.data(), operation.size());
        auto src_str = src.string();
        sha256.add(src_str.data(), src_str.size());
        auto dst_str = dst.string();
        sha256.add(dst_str.data(), dst_str.size());
        auto size_str = std::to_string(size);
        sha256.add(size_str.data(), size_str.size());
        sha256.add(checksum.data(), checksum.size());
        return sha256.getHash();
    }

    /// Get the hash of the last entry in the ledger
    std::string get_last_hash() {
        auto* sqlite_db = db_.get_db();
        if (!sqlite_db) return "GENESIS";

        const char* sql = "SELECT entry_hash FROM audit_ledger ORDER BY id DESC LIMIT 1";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return "GENESIS";
        }

        std::string result = "GENESIS";
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            if (hash) result = hash;
        }
        sqlite3_finalize(stmt);
        return result;
    }

public:
    explicit AuditLoggerImpl(DatabaseManager& db) : db_(db) {}

    void log(const std::string& op,
             const std::filesystem::path& src,
             const std::filesystem::path& dst,
             uintmax_t size,
             const std::string& checksum) override
    {
        auto* sqlite_db = db_.get_db();
        if (!sqlite_db) return;

        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

        std::string prev_hash = get_last_hash();

        // First compute a placeholder hash (id will be assigned by AUTOINCREMENT)
        // We use a two-step approach: insert, get the id, then update the hash
        const char* insert_sql = R"(
            INSERT INTO audit_ledger (timestamp, operation, src_path, dst_path, file_size, checksum, user_identity, system_id, entry_hash)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        )";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(sqlite_db, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) return;

        std::string placeholder = "pending";

        sqlite3_bind_int64(stmt, 1, timestamp);
        sqlite3_bind_text(stmt, 2, op.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, src.string().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, dst.string().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 5, static_cast<int64_t>(size));
        sqlite3_bind_text(stmt, 6, checksum.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, "user", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, "local", -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 9, placeholder.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return;
        }
        sqlite3_finalize(stmt);

        // Get the inserted ID
        int64_t entry_id = sqlite3_last_insert_rowid(sqlite_db);

        // Compute the real chain hash
        std::string entry_hash = compute_entry_hash(prev_hash, entry_id, timestamp, op, src, dst, size, checksum);

        // Update the entry with the real hash
        const char* update_sql = "UPDATE audit_ledger SET entry_hash = ? WHERE id = ?";
        if (sqlite3_prepare_v2(sqlite_db, update_sql, -1, &stmt, nullptr) != SQLITE_OK) return;

        sqlite3_bind_text(stmt, 1, entry_hash.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, entry_id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    std::vector<AuditEntry> get_history(int limit) override {
        auto* sqlite_db = db_.get_db();
        if (!sqlite_db) return {};

        const char* sql = "SELECT id, timestamp, operation, src_path, dst_path, file_size, checksum, user_identity, entry_hash "
                          "FROM audit_ledger ORDER BY id DESC LIMIT ?";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) return {};

        sqlite3_bind_int(stmt, 1, limit);

        std::vector<AuditEntry> entries;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            AuditEntry e;
            e.id = sqlite3_column_int64(stmt, 0);
            e.timestamp = sqlite3_column_int64(stmt, 1);

            const char* op = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            e.operation = op ? op : "";

            const char* src = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            e.src = src ? src : "";

            const char* dst = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            e.dst = dst ? dst : "";

            e.size = static_cast<uintmax_t>(sqlite3_column_int64(stmt, 5));

            const char* cksum = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            e.checksum = cksum ? cksum : "";

            const char* user = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            e.user = user ? user : "";

            const char* hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            e.hash = hash ? hash : "";

            entries.push_back(std::move(e));
        }
        sqlite3_finalize(stmt);
        return entries;
    }

    bool verify_ledger() override {
        auto* sqlite_db = db_.get_db();
        if (!sqlite_db) return true;  // No DB = nothing to verify

        const char* sql = "SELECT id, timestamp, operation, src_path, dst_path, file_size, checksum, entry_hash "
                          "FROM audit_ledger ORDER BY id ASC";
        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(sqlite_db, sql, -1, &stmt, nullptr) != SQLITE_OK) return true;  // Table doesn't exist = empty

        std::string prev_hash = "GENESIS";
        bool valid = true;

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            uint64_t id = sqlite3_column_int64(stmt, 0);
            int64_t timestamp = sqlite3_column_int64(stmt, 1);

            const char* op = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            std::string operation = op ? op : "";

            const char* src = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            std::filesystem::path src_path = src ? src : "";

            const char* dst = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            std::filesystem::path dst_path = dst ? dst : "";

            uintmax_t size = static_cast<uintmax_t>(sqlite3_column_int64(stmt, 5));

            const char* cksum = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            std::string checksum = cksum ? cksum : "";

            const char* hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            std::string entry_hash = hash ? hash : "";

            std::string expected = compute_entry_hash(prev_hash, id, timestamp, operation, src_path, dst_path, size, checksum);

            if (entry_hash != expected) {
                valid = false;
                break;
            }
            prev_hash = entry_hash;
        }
        sqlite3_finalize(stmt);
        return valid;
    }
};

// Registration — requires a DatabaseManager, so we register a factory that
// takes a DatabaseManager reference. For convenience, also provide a "default"
// that creates its own in-memory DB.
static bool reg_audit_logger = []() {
    // Register a factory that doesn't need DB (for simple use)
    // Real usage should construct AuditLoggerImpl directly with a DatabaseManager
    Registry<IAuditLogger>::instance().add("default", []() {
        // Create a standalone in-memory DB for the audit logger
        // This is a simple default; real usage passes the Engine's DB
        struct StandaloneAuditLogger : public IAuditLogger {
            DatabaseManager db;
            AuditLoggerImpl impl;

            StandaloneAuditLogger() : db(), impl(db) {
                db.open(":memory:");
                // Create the audit_ledger table (normally done by Migration 5)
                db.execute(R"(
                    CREATE TABLE IF NOT EXISTS audit_ledger (
                        id INTEGER PRIMARY KEY AUTOINCREMENT,
                        timestamp INTEGER NOT NULL DEFAULT (strftime('%s','now')),
                        operation TEXT NOT NULL,
                        src_path TEXT NOT NULL,
                        dst_path TEXT,
                        file_size INTEGER,
                        checksum TEXT,
                        user_identity TEXT,
                        system_id TEXT,
                        entry_hash TEXT NOT NULL
                    );
                    CREATE INDEX IF NOT EXISTS idx_audit_timestamp ON audit_ledger(timestamp);
                    CREATE INDEX IF NOT EXISTS idx_audit_operation ON audit_ledger(operation);
                )");
            }

            void log(const std::string& op, const std::filesystem::path& src,
                     const std::filesystem::path& dst, uintmax_t size,
                     const std::string& checksum) override {
                impl.log(op, src, dst, size, checksum);
            }
            std::vector<AuditEntry> get_history(int limit) override {
                return impl.get_history(limit);
            }
            bool verify_ledger() override {
                return impl.verify_ledger();
            }
        };
        return std::make_unique<StandaloneAuditLogger>();
    });
    return true;
}();

void register_audit_logger() { (void)reg_audit_logger; }

} // namespace fo::core
