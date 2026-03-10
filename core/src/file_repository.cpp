#include "fo/core/file_repository.hpp"
#include <sqlite3.h>
#include <stdexcept>
#include <iostream>
#include <bit>
#include <charconv>

namespace fo::core {

// Helper to convert file_time to unix timestamp (seconds)
static int64_t to_unix(std::chrono::file_clock::time_point tp) {
    // C++20 clock_cast
    // If this fails to compile on some platforms, we might need ifdefs.
    // For now, assuming C++20 compliant stdlib.
    try {
        auto sys = std::chrono::clock_cast<std::chrono::system_clock>(tp);
        return std::chrono::system_clock::to_time_t(sys);
    } catch (...) {
        return 0;
    }
}

// Helper to convert unix timestamp to file_time
static std::chrono::file_clock::time_point from_unix(int64_t t) {
    auto sys = std::chrono::system_clock::from_time_t(static_cast<std::time_t>(t));
    return std::chrono::clock_cast<std::chrono::file_clock>(sys);
}

FileRepository::FileRepository(DatabaseManager& db) : db_(db) {}

UpsertResult FileRepository::upsert(FileInfo& file) {
    UpsertResult result;
    auto existing = get_by_path(file.uri);
    
    if (!existing) {
        result.is_new = true;
        std::string sql = "INSERT INTO files (path, size, mtime, is_dir) VALUES (?, ?, ?, ?) RETURNING id;";
        
        sqlite3_stmt* stmt;
        int rc = sqlite3_prepare_v2(db_.get_db(), sql.c_str(), -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare insert: " + std::string(sqlite3_errmsg(db_.get_db())));
        }

        std::string path_str = file.uri;
        sqlite3_bind_text(stmt, 1, path_str.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(file.size));
        sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(to_unix(file.mtime)));
        sqlite3_bind_int(stmt, 4, file.is_dir ? 1 : 0);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            file.id = sqlite3_column_int64(stmt, 0);
        } else {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to execute insert: " + std::string(sqlite3_errmsg(db_.get_db())));
        }
        sqlite3_finalize(stmt);
    } else {
        file.id = existing->id;
        // Check if modified
        // Note: mtime comparison might be tricky due to precision.
        // We use second precision in DB (to_unix).
        int64_t new_mtime = to_unix(file.mtime);
        int64_t old_mtime = to_unix(existing->mtime);
        
        if (file.size != existing->size || new_mtime != old_mtime || file.is_dir != existing->is_dir) {
            result.is_modified = true;
            std::string sql = "UPDATE files SET size=?, mtime=?, is_dir=? WHERE id=?;";
            
            sqlite3_stmt* stmt;
            int rc = sqlite3_prepare_v2(db_.get_db(), sql.c_str(), -1, &stmt, nullptr);
            if (rc != SQLITE_OK) {
                throw std::runtime_error("Failed to prepare update: " + std::string(sqlite3_errmsg(db_.get_db())));
            }

            sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(file.size));
            sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(new_mtime));
            sqlite3_bind_int(stmt, 3, file.is_dir ? 1 : 0);
            sqlite3_bind_int64(stmt, 4, file.id);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                throw std::runtime_error("Failed to execute update: " + std::string(sqlite3_errmsg(db_.get_db())));
            }
            sqlite3_finalize(stmt);
        }
    }
    return result;
}

void FileRepository::prune_missing(const std::vector<int64_t>& present_ids, const std::vector<std::filesystem::path>& roots) {
    if (roots.empty()) return;

    // 1. Create temp table
    db_.execute("CREATE TEMPORARY TABLE IF NOT EXISTS present_files (id INTEGER PRIMARY KEY);");
    db_.execute("DELETE FROM present_files;");

    // 2. Insert IDs
    // db_.execute("BEGIN TRANSACTION;"); // Caller handles transaction
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_.get_db(), "INSERT INTO present_files (id) VALUES (?);", -1, &stmt, nullptr) == SQLITE_OK) {
        for (auto id : present_ids) {
            sqlite3_bind_int64(stmt, 1, id);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
        sqlite3_finalize(stmt);
    }
    // db_.execute("COMMIT;");

    // 3. Delete missing files under roots
    // db_.execute("BEGIN TRANSACTION;");
    sqlite3_stmt* del_stmt;
    std::string sql = "DELETE FROM files WHERE id NOT IN (SELECT id FROM present_files) AND (";
    for (size_t i = 0; i < roots.size(); ++i) {
        if (i > 0) sql += " OR ";
        sql += "path LIKE ? || '%'";
    }
    sql += ");";

    if (sqlite3_prepare_v2(db_.get_db(), sql.c_str(), -1, &del_stmt, nullptr) == SQLITE_OK) {
        for (size_t i = 0; i < roots.size(); ++i) {
            std::string root_str = roots[i].string();
            // Ensure root ends with separator to avoid partial matches (e.g. /foo matching /foobar)
            // But we need to be careful with root itself.
            // If root is "C:/foo", we want "C:/foo/bar" but also "C:/foo" itself?
            // Usually roots are directories.
            // Let's just use the path string.
            sqlite3_bind_text(del_stmt, static_cast<int>(i + 1), root_str.c_str(), -1, SQLITE_TRANSIENT);
        }
        sqlite3_step(del_stmt);
        sqlite3_finalize(del_stmt);
    }
    // db_.execute("COMMIT;");
    
    db_.execute("DROP TABLE present_files;");
}

void FileRepository::update_path(int64_t id, const std::filesystem::path& new_path) {
    std::string sql = "UPDATE files SET path = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_.get_db(), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) throw std::runtime_error("Failed to prepare update_path");

    std::string path_str = new_path.string();
    sqlite3_bind_text(stmt, 1, path_str.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to execute update_path");
    }
    sqlite3_finalize(stmt);
}

std::vector<FileInfo> FileRepository::get_missing_files(const std::vector<std::filesystem::path>& roots, const std::vector<int64_t>& present_ids) {
    std::vector<FileInfo> missing;
    if (roots.empty()) return missing;

    // 1. Create temp table and insert present_ids
    db_.execute("CREATE TEMPORARY TABLE IF NOT EXISTS present_files (id INTEGER PRIMARY KEY);");
    db_.execute("DELETE FROM present_files;");

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_.get_db(), "INSERT INTO present_files (id) VALUES (?);", -1, &stmt, nullptr) == SQLITE_OK) {
        for (auto id : present_ids) {
            sqlite3_bind_int64(stmt, 1, id);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
        sqlite3_finalize(stmt);
    }

    // 2. Select missing files
    std::string sql = "SELECT id, path, size, mtime, is_dir FROM files WHERE id NOT IN (SELECT id FROM present_files) AND (";
    for (size_t i = 0; i < roots.size(); ++i) {
        if (i > 0) sql += " OR ";
        sql += "path LIKE ? || '%'";
    }
    sql += ");";

    sqlite3_stmt* sel_stmt;
    if (sqlite3_prepare_v2(db_.get_db(), sql.c_str(), -1, &sel_stmt, nullptr) == SQLITE_OK) {
        for (size_t i = 0; i < roots.size(); ++i) {
            std::string root_str = roots[i].string();
            sqlite3_bind_text(sel_stmt, static_cast<int>(i + 1), root_str.c_str(), -1, SQLITE_TRANSIENT);
        }
        
        while (sqlite3_step(sel_stmt) == SQLITE_ROW) {
            FileInfo fi;
            fi.id = sqlite3_column_int64(sel_stmt, 0);
            const char* path_c = reinterpret_cast<const char*>(sqlite3_column_text(sel_stmt, 1));
            if (path_c) fi.uri = path_c;
            fi.size = static_cast<std::uintmax_t>(sqlite3_column_int64(sel_stmt, 2));
            fi.mtime = from_unix(sqlite3_column_int64(sel_stmt, 3));
            fi.is_dir = sqlite3_column_int(sel_stmt, 4) != 0;
            missing.push_back(fi);
        }
        sqlite3_finalize(sel_stmt);
    }
    
    db_.execute("DROP TABLE present_files;");
    return missing;
}

void FileRepository::delete_files(const std::vector<int64_t>& ids) {
    if (ids.empty()) return;
    
    db_.execute("CREATE TEMPORARY TABLE IF NOT EXISTS delete_ids (id INTEGER PRIMARY KEY);");
    db_.execute("DELETE FROM delete_ids;");
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_.get_db(), "INSERT INTO delete_ids (id) VALUES (?);", -1, &stmt, nullptr) == SQLITE_OK) {
        for (auto id : ids) {
            sqlite3_bind_int64(stmt, 1, id);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
        sqlite3_finalize(stmt);
    }
    
    db_.execute("DELETE FROM files WHERE id IN (SELECT id FROM delete_ids);");
    db_.execute("DROP TABLE delete_ids;");
}

std::optional<FileInfo> FileRepository::get_by_path(const std::filesystem::path& path) {
    std::string sql = "SELECT id, size, mtime, is_dir FROM files WHERE path = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_.get_db(), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return std::nullopt;

    std::string path_str = path.string();
    sqlite3_bind_text(stmt, 1, path_str.c_str(), -1, SQLITE_STATIC);

    std::optional<FileInfo> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        FileInfo fi;
        fi.uri = path.string();
        fi.id = sqlite3_column_int64(stmt, 0);
        fi.size = static_cast<std::uintmax_t>(sqlite3_column_int64(stmt, 1));
        fi.mtime = from_unix(sqlite3_column_int64(stmt, 2));
        fi.is_dir = sqlite3_column_int(stmt, 3) != 0;
        result = fi;
    }
    sqlite3_finalize(stmt);
    return result;
}

void FileRepository::add_hash(int64_t file_id, const std::string& algo, const std::string& value) {
    std::string sql = "INSERT INTO file_hashes (file_id, algo, value) VALUES (?, ?, ?) "
                      "ON CONFLICT(file_id, algo) DO UPDATE SET value=excluded.value;";
    
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_.get_db(), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) throw std::runtime_error("Prepare failed");

    sqlite3_bind_int64(stmt, 1, file_id);
    sqlite3_bind_text(stmt, 2, algo.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, value.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.get_db());
        sqlite3_finalize(stmt);
        throw std::runtime_error("Failed to add hash: " + err);
    }
    sqlite3_finalize(stmt);
}

std::vector<std::pair<std::string, std::string>> FileRepository::get_hashes(int64_t file_id) {
    std::vector<std::pair<std::string, std::string>> out;
    std::string sql = "SELECT algo, value FROM file_hashes WHERE file_id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_.get_db(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return out;

    sqlite3_bind_int64(stmt, 1, file_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string algo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string val = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        out.emplace_back(algo, val);
    }
    sqlite3_finalize(stmt);
    return out;
}

std::optional<FileInfo> FileRepository::get_by_id(int64_t id) {
    std::string sql = "SELECT path, size, mtime, is_dir FROM files WHERE id = ?;";
    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db_.get_db(), sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return std::nullopt;

    sqlite3_bind_int64(stmt, 1, id);

    std::optional<FileInfo> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        FileInfo fi;
        fi.id = id;
        const char* path_c = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (path_c) fi.uri = path_c;
        fi.size = static_cast<std::uintmax_t>(sqlite3_column_int64(stmt, 1));
        fi.mtime = from_unix(sqlite3_column_int64(stmt, 2));
        fi.is_dir = sqlite3_column_int(stmt, 3) != 0;
        result = fi;
    }
    sqlite3_finalize(stmt);
    return result;
}

std::vector<int64_t> FileRepository::find_similar_images(uint64_t target_hash, int threshold) {
    std::vector<int64_t> matches;
    // Select all dhash values
    std::string sql = "SELECT file_id, value FROM file_hashes WHERE algo = 'dhash';";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_.get_db(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return matches;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int64_t file_id = sqlite3_column_int64(stmt, 0);
        const char* val_c = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        if (!val_c) continue;

        uint64_t hash = 0;
        // Parse string to uint64
        // Assuming decimal string from std::to_string
        std::from_chars(val_c, val_c + strlen(val_c), hash);

        int dist = std::popcount(target_hash ^ hash);
        if (dist <= threshold) {
            matches.push_back(file_id);
        }
    }
    sqlite3_finalize(stmt);
    return matches;
}

void FileRepository::add_tag(int64_t file_id, const std::string& tag, double confidence, const std::string& source) {
    // 1. Ensure tag exists
    std::string sql_tag = "INSERT INTO tags (name) VALUES (?) ON CONFLICT(name) DO UPDATE SET name=name RETURNING id;";
    sqlite3_stmt* stmt_tag;
    if (sqlite3_prepare_v2(db_.get_db(), sql_tag.c_str(), -1, &stmt_tag, nullptr) != SQLITE_OK) return;
    
    sqlite3_bind_text(stmt_tag, 1, tag.c_str(), -1, SQLITE_STATIC);
    
    int64_t tag_id = 0;
    if (sqlite3_step(stmt_tag) == SQLITE_ROW) {
        tag_id = sqlite3_column_int64(stmt_tag, 0);
    }
    sqlite3_finalize(stmt_tag);
    
    if (tag_id == 0) {
        // Fallback: select id if insert failed (shouldn't happen with RETURNING but just in case)
        std::string sql_sel = "SELECT id FROM tags WHERE name = ?;";
        sqlite3_stmt* stmt_sel;
        sqlite3_prepare_v2(db_.get_db(), sql_sel.c_str(), -1, &stmt_sel, nullptr);
        sqlite3_bind_text(stmt_sel, 1, tag.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt_sel) == SQLITE_ROW) tag_id = sqlite3_column_int64(stmt_sel, 0);
        sqlite3_finalize(stmt_sel);
    }
    
    if (tag_id == 0) return; // Failed to get tag ID

    // 2. Link file and tag
    std::string sql_link = "INSERT INTO file_tags (file_id, tag_id, confidence, source) VALUES (?, ?, ?, ?) "
                           "ON CONFLICT(file_id, tag_id) DO UPDATE SET confidence=excluded.confidence, source=excluded.source;";
    
    sqlite3_stmt* stmt_link;
    if (sqlite3_prepare_v2(db_.get_db(), sql_link.c_str(), -1, &stmt_link, nullptr) != SQLITE_OK) return;

    sqlite3_bind_int64(stmt_link, 1, file_id);
    sqlite3_bind_int64(stmt_link, 2, tag_id);
    sqlite3_bind_double(stmt_link, 3, confidence);
    sqlite3_bind_text(stmt_link, 4, source.c_str(), -1, SQLITE_STATIC);

    sqlite3_step(stmt_link);
    sqlite3_finalize(stmt_link);
}

std::vector<std::pair<std::string, double>> FileRepository::get_tags(int64_t file_id) {
    std::vector<std::pair<std::string, double>> out;
    std::string sql = "SELECT t.name, ft.confidence FROM file_tags ft "
                      "JOIN tags t ON ft.tag_id = t.id "
                      "WHERE ft.file_id = ? ORDER BY ft.confidence DESC;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_.get_db(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return out;

    sqlite3_bind_int64(stmt, 1, file_id);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        double conf = sqlite3_column_double(stmt, 1);
        out.emplace_back(name, conf);
    }
    sqlite3_finalize(stmt);
    return out;
}

} // namespace fo::core
