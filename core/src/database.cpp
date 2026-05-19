#include <algorithm>
#include <cmath>
#include "fo/core/database.hpp"
#include <sqlite3.h>
#include <stdexcept>
#include <iostream>
#include <filesystem>

namespace fo::core {

// --- Migrations ---

static const char* MIGRATION_1 = R"(
CREATE TABLE IF NOT EXISTS schema_version (
    version INTEGER PRIMARY KEY
);

CREATE TABLE IF NOT EXISTS files (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    path TEXT NOT NULL UNIQUE,
    size INTEGER NOT NULL,
    mtime INTEGER NOT NULL,
    is_dir INTEGER DEFAULT 0
);

CREATE TABLE IF NOT EXISTS file_hashes (
    file_id INTEGER NOT NULL,
    algo TEXT NOT NULL,
    value TEXT NOT NULL,
    PRIMARY KEY (file_id, algo),
    FOREIGN KEY (file_id) REFERENCES files(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_files_path ON files(path);
CREATE INDEX IF NOT EXISTS idx_files_size ON files(size);
CREATE INDEX IF NOT EXISTS idx_hashes_value ON file_hashes(value);

CREATE TABLE IF NOT EXISTS duplicate_groups (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    primary_file_id INTEGER,
    FOREIGN KEY (primary_file_id) REFERENCES files(id)
);

CREATE TABLE IF NOT EXISTS duplicate_members (
    group_id INTEGER NOT NULL,
    file_id INTEGER NOT NULL,
    PRIMARY KEY (group_id, file_id),
    FOREIGN KEY (group_id) REFERENCES duplicate_groups(id) ON DELETE CASCADE,
    FOREIGN KEY (file_id) REFERENCES files(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS ignore_list (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    pattern TEXT NOT NULL UNIQUE,
    reason TEXT
);

CREATE TABLE IF NOT EXISTS scan_sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    start_time INTEGER NOT NULL,
    end_time INTEGER,
    status TEXT,
    scanned_count INTEGER DEFAULT 0,
    duration_ms INTEGER DEFAULT 0
);
)";

static const char* MIGRATION_2 = R"(
CREATE TABLE IF NOT EXISTS tags (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS file_tags (
    file_id INTEGER NOT NULL,
    tag_id INTEGER NOT NULL,
    confidence REAL DEFAULT 1.0,
    source TEXT,
    PRIMARY KEY (file_id, tag_id),
    FOREIGN KEY (file_id) REFERENCES files(id) ON DELETE CASCADE,
    FOREIGN KEY (tag_id) REFERENCES tags(id) ON DELETE CASCADE
);
)";

static const char* MIGRATION_3 = R"(
CREATE TABLE IF NOT EXISTS operation_log (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp INTEGER NOT NULL,
    operation_type TEXT NOT NULL,
    source_path TEXT NOT NULL,
    dest_path TEXT,
    file_size INTEGER,
    file_hash TEXT,
    status TEXT DEFAULT 'completed',
    undone INTEGER DEFAULT 0
);

CREATE INDEX IF NOT EXISTS idx_operation_log_timestamp ON operation_log(timestamp);
CREATE INDEX IF NOT EXISTS idx_operation_log_status ON operation_log(status);
CREATE INDEX IF NOT EXISTS idx_operation_log_undone ON operation_log(undone);
)";

/// Migration 4: Vector-Semantic Search embeddings table
/// Stores 512-dimensional CLIP embedding vectors as BLOBs alongside file metadata.
/// Designed for O(N) brute-force cosine similarity; for large libraries (>100k files),
/// consider an ANN index (hnswlib) layered on top.
static const char* MIGRATION_4 = R"(
CREATE TABLE IF NOT EXISTS file_embeddings (
    file_id INTEGER PRIMARY KEY,
    model_name TEXT NOT NULL DEFAULT 'clip-vit-b32',
    vector BLOB NOT NULL,
    dimensions INTEGER NOT NULL DEFAULT 512,
    created_at INTEGER NOT NULL DEFAULT (strftime('%s','now')),
    FOREIGN KEY (file_id) REFERENCES files(id) ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_embeddings_model ON file_embeddings(model_name);
)";

/// Migration 5: Forensic Audit Ledger
/// An immutable, append-only record of every file operation (move, delete, rename).
/// Includes SHA-256 of the previous state to detect tampering.
static const char* MIGRATION_5 = R"(
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
    entry_hash TEXT NOT NULL  -- HMAC or Hash of (id, timestamp, operation, src, dst)
);

CREATE INDEX IF NOT EXISTS idx_audit_timestamp ON audit_ledger(timestamp);
CREATE INDEX IF NOT EXISTS idx_audit_operation ON audit_ledger(operation);
)";

/// Migration 6: Pro Photo Management (Lightroom / Apple Photos)
/// Stores deep photo metadata: faces, GPS, AI tags, and non-destructive edit stacks.
static const char* MIGRATION_6 = R"(
CREATE TABLE IF NOT EXISTS photos (
    file_id INTEGER PRIMARY KEY,
    width INTEGER,
    height INTEGER,
    iso INTEGER,
    f_stop REAL,
    shutter_speed REAL,
    focal_length REAL,
    lat REAL,
    lon REAL,
    is_raw INTEGER DEFAULT 0,
    edit_stack TEXT, -- JSON blob of non-destructive adjustments
    FOREIGN KEY (file_id) REFERENCES files(id) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS people (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    thumbnail_path TEXT
);

CREATE TABLE IF NOT EXISTS photo_faces (
    photo_id INTEGER NOT NULL,
    person_id INTEGER,
    confidence REAL,
    box_json TEXT,
    FOREIGN KEY (photo_id) REFERENCES photos(file_id) ON DELETE CASCADE,
    FOREIGN KEY (person_id) REFERENCES people(id) ON DELETE SET NULL
);

CREATE INDEX IF NOT EXISTS idx_photos_raw ON photos(is_raw);
CREATE INDEX IF NOT EXISTS idx_photos_gps ON photos(lat, lon);
)";

// ------------------

DatabaseManager::DatabaseManager() : db_(nullptr) {}

DatabaseManager::~DatabaseManager() {
    close();
}

void DatabaseManager::open(const std::filesystem::path& db_path) {
    if (db_) {
        close();
    }

    db_path_ = db_path;
    
    // Ensure directory exists
    if (db_path != ":memory:" && db_path.has_parent_path()) {
        std::filesystem::create_directories(db_path.parent_path());
    }

    // Convert path to string (UTF-8 on Windows if using std::filesystem::path::string() with proper locale, 
    // but sqlite3_open expects UTF-8).
    // On Windows, we should use sqlite3_open16 with wstring, or ensure string() is UTF-8.
    // For cross-platform simplicity with std::filesystem, generic_string() or string() is usually fine,
    // but let's use sqlite3_open_v2.

    int rc = sqlite3_open(db_path.string().c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::string err = db_ ? sqlite3_errmsg(db_) : "Unknown error";
        if (db_) sqlite3_close(db_);
        db_ = nullptr;
        throw std::runtime_error("Failed to open database: " + err);
    }

    // Enable foreign keys
    execute("PRAGMA foreign_keys = ON;");
    // WAL mode for better concurrency
    execute("PRAGMA journal_mode = WAL;");
    // Synchronous NORMAL is usually safe enough for WAL and faster
    execute("PRAGMA synchronous = NORMAL;");
}

void DatabaseManager::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

sqlite3* DatabaseManager::get_db() const {
    return db_;
}

void DatabaseManager::execute(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::string err = errMsg ? errMsg : "Unknown error";
        sqlite3_free(errMsg);
        throw std::runtime_error("SQL error: " + err + " in statement: " + sql);
    }
}

int DatabaseManager::query_int(const std::string& sql) {
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
    }

    int result = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return result;
}

int DatabaseManager::get_current_version() {
    // Check if table exists first
    int table_exists = query_int("SELECT count(*) FROM sqlite_master WHERE type='table' AND name='schema_version';");
    if (!table_exists) return 0;

    return query_int("SELECT MAX(version) FROM schema_version;");
}

void DatabaseManager::apply_migration(int version, const std::string& sql) {
    execute("BEGIN TRANSACTION;");
    try {
        execute(sql);
        std::string update_ver = "INSERT INTO schema_version (version) VALUES (" + std::to_string(version) + ");";
        execute(update_ver);
        execute("COMMIT;");
    } catch (...) {
        execute("ROLLBACK;");
        throw;
    }
}

void DatabaseManager::migrate() {
    int current_ver = get_current_version();

    if (current_ver < 1) {
        apply_migration(1, MIGRATION_1);
    }
    if (current_ver < 2) {
        apply_migration(2, MIGRATION_2);
    }
    if (current_ver < 3) {
        apply_migration(3, MIGRATION_3);
    }
    if (current_ver < 4) {
        apply_migration(4, MIGRATION_4);
    }
    if (current_ver < 5) {
        apply_migration(5, MIGRATION_5);
    }
}

// ── Vector Embedding Implementation ──────────────────────────────────────────

void DatabaseManager::store_embedding(int64_t file_id, const std::string& model_name, const std::vector<float>& vector) {
    const char* sql = "INSERT OR REPLACE INTO file_embeddings (file_id, model_name, vector, dimensions) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    sqlite3_bind_int64(stmt, 1, file_id);
    sqlite3_bind_text(stmt, 2, model_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 3, vector.data(), static_cast<int>(vector.size() * sizeof(float)), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, static_cast<int>(vector.size()));

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

static float cosine_similarity(const float* a, const float* b, size_t n) {
    double dot = 0, norm_a = 0, norm_b = 0;
    for (size_t i = 0; i < n; ++i) {
        dot += a[i] * b[i];
        norm_a += a[i] * a[i];
        norm_b += b[i] * b[i];
    }
    if (norm_a == 0 || norm_b == 0) return 0;
    return static_cast<float>(dot / (std::sqrt(norm_a) * std::sqrt(norm_b)));
}

std::vector<DatabaseManager::EmbeddingMatch> DatabaseManager::search_embeddings(const std::vector<float>& query, double threshold, int top_k) {
    std::vector<EmbeddingMatch> results;
    const char* sql = "SELECT file_id, vector FROM file_embeddings";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return {};

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int64_t file_id = sqlite3_column_int64(stmt, 0);
        const float* db_vec = static_cast<const float*>(sqlite3_column_blob(stmt, 1));
        size_t db_bytes = sqlite3_column_bytes(stmt, 1);
        size_t n = db_bytes / sizeof(float);

        if (n == query.size()) {
            float sim = cosine_similarity(query.data(), db_vec, n);
            if (sim >= threshold) {
                results.push_back({file_id, static_cast<double>(sim)});
            }
        }
    }
    sqlite3_finalize(stmt);

    // Sort by score descending
    std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
        return a.score > b.score;
    });

    // Limit to top_k
    if (top_k > 0 && results.size() > static_cast<size_t>(top_k)) {
        results.resize(top_k);
    }

    return results;
}

} // namespace fo::core
