# Database Schema Design

filez uses **SQLite3** for local, embedded persistence. The schema supports:
- File metadata (paths, sizes, timestamps, hashes)
- EXIF/date information
- Duplicate grouping
- Ignore lists (directories/patterns to skip)
- Incremental scanning (track scan sessions)
- Migration versioning

---

## Schema Overview

### Tables

1. **`schema_version`**: Track migration history
2. **`files`**: Core file metadata (path, size, mtime, hashes)
3. **`file_dates`**: Normalized date metadata (EXIF, filename-parsed, etc.)
4. **`file_exif`**: Detailed EXIF tags (optional; for queryable metadata)
5. **`duplicate_groups`**: Clustered duplicates
6. **`duplicate_members`**: Many-to-many link between files and groups
7. **`ignore_list`**: Directories/patterns to exclude from scans
8. **`scan_sessions`**: Track scan runs for incremental updates
9. **`ocr_results`**: OCR text extracted from images (optional feature)
10. **`perceptual_hashes`**: Image similarity hashes (pHash, dHash, etc.)

---

## Table Definitions

### 1. `schema_version`

Tracks applied migrations for schema evolution.

```sql
CREATE TABLE schema_version (
    version INTEGER PRIMARY KEY,
    applied_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    description TEXT
);

-- Initial migration
INSERT INTO schema_version (version, description) VALUES (1, 'Initial schema');
```

---

### 2. `files`

Core file metadata. Use normalized paths (absolute, case-preserved).

```sql
CREATE TABLE files (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    path TEXT NOT NULL UNIQUE COLLATE NOCASE, -- Case-insensitive for Windows compat
    size INTEGER NOT NULL,                     -- Bytes
    mtime_epoch INTEGER NOT NULL,              -- Modification time (Unix epoch)
    fast_hash TEXT,                            -- Prefilter hash (XXH3, fast64)
    strong_hash TEXT,                          -- Verification hash (SHA256, BLAKE3)
    strong_hash_algo TEXT,                     -- Algorithm name (for multi-algo support)
    has_exif BOOLEAN DEFAULT 0,                -- Flag: EXIF data available
    has_gps BOOLEAN DEFAULT 0,                 -- Flag: GPS coordinates present
    has_ocr BOOLEAN DEFAULT 0,                 -- Flag: OCR text extracted
    last_scanned_session INTEGER,              -- FK to scan_sessions
    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_files_path ON files(path);
CREATE INDEX idx_files_size ON files(size);
CREATE INDEX idx_files_fast_hash ON files(fast_hash);
CREATE INDEX idx_files_strong_hash ON files(strong_hash);
CREATE INDEX idx_files_mtime ON files(mtime_epoch);
```

---

### 3. `file_dates`

Normalized date/time metadata from multiple sources (priority order).

```sql
CREATE TABLE file_dates (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    file_id INTEGER NOT NULL,
    source TEXT NOT NULL,                      -- 'EXIF:DateTimeOriginal', 'Filename', 'FileMtime', etc.
    date_epoch INTEGER,                        -- Unix epoch (UTC)
    date_string TEXT,                          -- Original string (for logging/debugging)
    confidence INTEGER DEFAULT 50,             -- 0-100: how reliable is this date?
    is_primary BOOLEAN DEFAULT 0,              -- Flag: primary date chosen for this file
    FOREIGN KEY (file_id) REFERENCES files(id) ON DELETE CASCADE
);

CREATE INDEX idx_file_dates_file_id ON file_dates(file_id);
CREATE INDEX idx_file_dates_date_epoch ON file_dates(date_epoch);
CREATE INDEX idx_file_dates_is_primary ON file_dates(is_primary);
```

**Confidence heuristic**:
- EXIF DateTimeOriginal: 100
- EXIF DateTimeDigitized: 90
- EXIF DateTime: 70
- Filename (parsed): 60
- File mtime: 30

---

### 4. `file_exif` (Optional: for advanced querying)

Store detailed EXIF tags for searching/filtering.

```sql
CREATE TABLE file_exif (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    file_id INTEGER NOT NULL,
    make TEXT,
    model TEXT,
    lens TEXT,
    focal_length REAL,
    aperture REAL,
    shutter_speed REAL,
    iso INTEGER,
    gps_lat REAL,
    gps_lon REAL,
    orientation INTEGER,
    FOREIGN KEY (file_id) REFERENCES files(id) ON DELETE CASCADE
);

CREATE INDEX idx_file_exif_file_id ON file_exif(file_id);
CREATE INDEX idx_file_exif_make_model ON file_exif(make, model);
CREATE INDEX idx_file_exif_gps ON file_exif(gps_lat, gps_lon);
```

---

### 5. `duplicate_groups`

Groups of potential duplicates (size + fast_hash match).

```sql
CREATE TABLE duplicate_groups (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    size INTEGER NOT NULL,
    fast_hash TEXT NOT NULL,
    strong_hash TEXT,                          -- Optional: verified strong hash if all members match
    member_count INTEGER NOT NULL DEFAULT 2,
    verified BOOLEAN DEFAULT 0,                -- 1 = byte-compare or strong hash verified
    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_duplicate_groups_size_hash ON duplicate_groups(size, fast_hash);
CREATE INDEX idx_duplicate_groups_verified ON duplicate_groups(verified);
```

---

### 6. `duplicate_members`

Link table: files ↔ duplicate_groups (many-to-many).

```sql
CREATE TABLE duplicate_members (
    group_id INTEGER NOT NULL,
    file_id INTEGER NOT NULL,
    is_keeper BOOLEAN DEFAULT NULL,            -- User decision: keep/delete/undecided
    PRIMARY KEY (group_id, file_id),
    FOREIGN KEY (group_id) REFERENCES duplicate_groups(id) ON DELETE CASCADE,
    FOREIGN KEY (file_id) REFERENCES files(id) ON DELETE CASCADE
);

CREATE INDEX idx_duplicate_members_file_id ON duplicate_members(file_id);
CREATE INDEX idx_duplicate_members_group_id ON duplicate_members(group_id);
```

---

### 7. `ignore_list`

Directories and patterns to exclude from scans.

```sql
CREATE TABLE ignore_list (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    pattern TEXT NOT NULL UNIQUE,              -- Glob or regex pattern
    pattern_type TEXT NOT NULL DEFAULT 'glob', -- 'glob', 'regex', 'exact_dir'
    reason TEXT,                               -- User note
    created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_ignore_list_pattern ON ignore_list(pattern);
```

**Examples**:
- `/mnt/backup/*` (glob)
- `.*\.tmp$` (regex)
- `C:\Windows\System32` (exact_dir)

---

### 8. `scan_sessions`

Track scan runs for incremental updates and audit logs.

```sql
CREATE TABLE scan_sessions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    started_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    completed_at TEXT,
    root_paths TEXT NOT NULL,                  -- JSON array of scanned roots
    scanner_name TEXT,                         -- 'std', 'dirent', etc.
    hasher_name TEXT,                          -- 'xxh3', 'sha256', etc.
    files_scanned INTEGER DEFAULT 0,
    duplicates_found INTEGER DEFAULT 0,
    status TEXT DEFAULT 'running'              -- 'running', 'completed', 'failed'
);

CREATE INDEX idx_scan_sessions_started_at ON scan_sessions(started_at);
```

---

### 9. `ocr_results` (Optional: for text search)

Store OCR text extracted from images.

```sql
CREATE TABLE ocr_results (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    file_id INTEGER NOT NULL,
    text TEXT NOT NULL,
    confidence REAL,                           -- 0.0–1.0
    lang TEXT DEFAULT 'eng',
    provider TEXT,                             -- 'tesseract', 'paddleocr', etc.
    extracted_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (file_id) REFERENCES files(id) ON DELETE CASCADE
);

CREATE INDEX idx_ocr_results_file_id ON ocr_results(file_id);
CREATE VIRTUAL TABLE ocr_text_fts USING fts5(file_id, text, content=ocr_results);
```

**Full-text search** via FTS5 for fast keyword queries.

---

### 10. `perceptual_hashes` (Optional: for image similarity)

Store perceptual hashes (pHash, dHash) for near-duplicate detection.

```sql
CREATE TABLE perceptual_hashes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    file_id INTEGER NOT NULL,
    hash_type TEXT NOT NULL,                   -- 'phash', 'dhash', 'ahash', 'pdq'
    hash_value TEXT NOT NULL,                  -- Hex string (64-bit or 256-bit)
    computed_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (file_id) REFERENCES files(id) ON DELETE CASCADE
);

CREATE INDEX idx_perceptual_hashes_file_id ON perceptual_hashes(file_id);
CREATE INDEX idx_perceptual_hashes_type_value ON perceptual_hashes(hash_type, hash_value);
```

**Query strategy**: For similarity search, compute Hamming distance between query hash and all hashes of the same type. Use threshold (e.g., ≤5 bits for near-duplicates).

---

## Migration Strategy

Use a simple migration system with numbered SQL scripts.

### Migration Manager (C++)

```cpp
#include <sqlite3.h>
#include <vector>
#include <string>

struct Migration {
    int version;
    std::string description;
    std::string sql;
};

std::vector<Migration> get_migrations() {
    return {
        {1, "Initial schema", R"SQL(
            -- All CREATE TABLE statements from above
        )SQL"},
        {2, "Add perceptual_hashes table", R"SQL(
            CREATE TABLE perceptual_hashes (...);
        )SQL"},
        // Add more as schema evolves
    };
}

void apply_migrations(sqlite3* db) {
    // Get current version
    int current_version = 0;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, "SELECT MAX(version) FROM schema_version", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            current_version = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    // Apply pending migrations
    for (auto& m : get_migrations()) {
        if (m.version <= current_version) continue;
        sqlite3_exec(db, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
        if (sqlite3_exec(db, m.sql.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK) {
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            throw std::runtime_error("Migration " + std::to_string(m.version) + " failed");
        }
        std::string insert = "INSERT INTO schema_version (version, description) VALUES (" 
                           + std::to_string(m.version) + ", '" + m.description + "')";
        sqlite3_exec(db, insert.c_str(), nullptr, nullptr, nullptr);
        sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
    }
}
```

---

## Usage Examples

### Insert a scanned file

```sql
INSERT INTO files (path, size, mtime_epoch, fast_hash, strong_hash, strong_hash_algo)
VALUES ('/photos/IMG_1234.jpg', 2048576, 1699900800, 'a1b2c3d4e5f60718', 'sha256:abcdef...', 'SHA-256');

-- Add primary date from EXIF
INSERT INTO file_dates (file_id, source, date_epoch, date_string, confidence, is_primary)
VALUES (last_insert_rowid(), 'EXIF:DateTimeOriginal', 1699900800, '2023:11:14 10:30:00', 100, 1);
```

### Find all duplicates

```sql
SELECT g.id, g.size, g.fast_hash, g.member_count, f.path
FROM duplicate_groups g
JOIN duplicate_members dm ON g.id = dm.group_id
JOIN files f ON dm.file_id = f.id
WHERE g.verified = 1
ORDER BY g.size DESC, g.id, f.path;
```

### Search OCR text

```sql
SELECT f.path, ocr.text
FROM ocr_text_fts
JOIN files f ON ocr_text_fts.file_id = f.id
WHERE ocr_text_fts MATCH 'invoice OR receipt'
ORDER BY rank;
```

### Find similar images (perceptual hash)

```sql
-- Pseudo-query (requires custom function for Hamming distance)
SELECT f.path, hamming_distance(ph.hash_value, ?query_hash) AS distance
FROM perceptual_hashes ph
JOIN files f ON ph.file_id = f.id
WHERE ph.hash_type = 'phash' AND distance <= 5
ORDER BY distance;
```

---

## Alternate Data Streams (Windows)

On Windows (NTFS), you can store hashes/metadata directly with files using Alternate Data Streams (ADS). This avoids a central DB but is Windows-only.

**Pros**:
- No DB file; metadata travels with files when moved (on same NTFS volume)
- Fast incremental scans (check ADS timestamp vs. file mtime)

**Cons**:
- NTFS-only; lost on FAT32/exFAT or network shares
- Not queryable via SQL
- Less robust for grouping duplicates

**Recommendation**: Use ADS as an **optional cache** for fast hashes on Windows, with SQLite as the primary source of truth. On startup, check ADS for existing hashes; fall back to DB if missing.

**Example (C++)**:
```cpp
#include <windows.h>
#include <fstream>

void write_ads_hash(const std::wstring& path, const std::string& hash) {
    std::wstring ads_path = path + L":fo_hash";
    std::ofstream f(ads_path, std::ios::binary);
    f << hash;
}

std::string read_ads_hash(const std::wstring& path) {
    std::wstring ads_path = path + L":fo_hash";
    std::ifstream f(ads_path, std::ios::binary);
    if (!f) return {};
    std::string hash;
    std::getline(f, hash);
    return hash;
}
```

---

## Next Steps

1. **Implement migration manager**: C++ helper to apply schema updates on DB open.
2. **Add data-access layer**: Classes like `FileRepository`, `DuplicateRepository` to abstract SQL.
3. **Integrate with Engine**: Update `Engine::scan()` to insert/update `files` and `file_dates`; populate `duplicate_groups`.
4. **CLI commands**: `fo_cli scan`, `fo_cli duplicates`, `fo_cli ocr`, `fo_cli search`.
5. **ADS integration**: Optional flag `--use-ads-cache` to read/write NTFS streams on Windows.
6. **Backup/export**: `fo_cli export` to dump DB as JSON/CSV for portability.
7. **GUI sync**: IPC protocol to update UI (via WebSocket, named pipe, or shared memory) when DB changes.

---

## Summary

This schema design:
- Supports all core features (scanning, hashing, metadata, duplicates, OCR, perceptual hashing)
- Enables incremental scanning and audit trails
- Provides migration versioning for schema evolution
- Balances normalization (queryability) with denormalization (performance)
- Offers ADS as an optional Windows-specific optimization

SQLite's FTS5 and custom functions (Hamming distance) enable advanced queries without external search engines.
