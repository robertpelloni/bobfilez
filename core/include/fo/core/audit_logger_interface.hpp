#pragma once
/// @file audit_logger_interface.hpp
/// @brief Forensic Audit Ledger for bobfilez.
///
/// Provides an immutable record of all destructive or movement-based file 
/// operations. Used for corporate compliance, forensic recovery, and 
/// multi-user environments.

#include <string>
#include <filesystem>
#include <memory>
#include <vector>

namespace fo::core {

class DatabaseManager;

struct AuditEntry {
    uint64_t id;
    int64_t timestamp;
    std::string operation;
    std::filesystem::path src;
    std::filesystem::path dst;
    uintmax_t size;
    std::string checksum;
    std::string user;
    std::string hash; // Self-tamper check
};

class IAuditLogger {
public:
    virtual ~IAuditLogger() = default;

    /// Log a new operation to the immutable ledger
    virtual void log(const std::string& op, 
                     const std::filesystem::path& src, 
                     const std::filesystem::path& dst = "", 
                     uintmax_t size = 0, 
                     const std::string& checksum = "") = 0;

    /// Retrieve the last N entries
    virtual std::vector<AuditEntry> get_history(int limit = 100) = 0;

    /// Verify the integrity of the ledger
    virtual bool verify_ledger() = 0;
};

/// Create an audit logger backed by a persistent DatabaseManager (the
/// Engine's database). This is the production counterpart to the
/// registry's in-memory "default" factory, which exists only for tests.
std::unique_ptr<IAuditLogger> make_audit_logger(DatabaseManager& db);

} // namespace fo::core
