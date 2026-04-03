#pragma once

#include <string>
#include <memory>
#include <vector>
#include <filesystem>

// Forward declaration to avoid exposing sqlite3.h in the header
struct sqlite3;

namespace fo::core {

class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    // Disable copy/move
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // Open the database at the specified path.
    // If path is ":memory:", opens an in-memory database.
    void open(const std::filesystem::path& db_path);

    // Close the database connection.
    void close();

    // Apply pending migrations to bring the schema up to date.
    void migrate();

    // Get the raw sqlite3 handle.
    sqlite3* get_db() const;

    // Helper to execute a simple SQL statement (no results).
    void execute(const std::string& sql);

    // Helper to execute a scalar query (returns int).
    int query_int(const std::string& sql);

    // ── Vector Embedding Support ───────────────────────────────────────────
    
    struct EmbeddingMatch { int64_t file_id; double score; };

    /// Store a float vector as a BLOB in the file_embeddings table.
    void store_embedding(int64_t file_id, 
                         const std::string& model_name, 
                         const std::vector<float>& vector);

    /// Retrieve and compute similarity for all stored embeddings.
    std::vector<EmbeddingMatch> search_embeddings(const std::vector<float>& query_vector, 
                                                  double threshold = 0.2, 
                                                  int top_k = 100);

private:
    sqlite3* db_ = nullptr;
    std::filesystem::path db_path_;

    void apply_migration(int version, const std::string& sql);
    int get_current_version();
};

} // namespace fo::core
