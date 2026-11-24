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
    // treated as its own batch. Last field on purpose: aggregate
    // initializers elsewhere must not shift.
    int64_t batch_id = 0;
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

    // Allocate a fresh batch id for one multi-file command.
    int64_t allocate_batch_id();

    // Undo the most recent undoable operation
    std::optional<OperationRecord> undo_last();

    // Undo every undoable record in the most recent batch (the
    // batch of the newest undoable record), newest first. Stale
    // records are skipped; a batch that yields nothing falls
    // through to the next older batch, mirroring undo_last.
    std::vector<OperationRecord> undo_last_batch();

    // Get operation by ID
    std::optional<OperationRecord> get_by_id(int64_t id);

    // Clear old operations (older than days)
    void clear_old(int days = 30);

private:
    DatabaseManager& db_;
    
    static std::string operation_type_to_string(OperationType type);
    static OperationType string_to_operation_type(const std::string& str);
};

} // namespace fo::core

