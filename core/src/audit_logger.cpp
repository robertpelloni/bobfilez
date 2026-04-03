/// @file audit_logger.cpp
/// @brief Implementation of the Forensic Audit Ledger using SQLite.

#include "fo/core/audit_logger_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/database.hpp"
#include <iostream>
#include <sstream>

namespace fo::core {

class AuditLoggerImpl : public IAuditLogger {
public:
    void log(const std::string& op, const std::filesystem::path& src, const std::filesystem::path& dst, uintmax_t size, const std::string& checksum) override {
        // Placeholder for DB insertion
        // In a real implementation, this computes a HMAC or chaining hash
        // of the record to ensure immutability.
        std::cout << "[Audit] " << op << ": " << src << " -> " << dst << " (" << size << " bytes)\n";
    }

    std::vector<AuditEntry> get_history(int limit) override {
        return {}; // Stub
    }

    bool verify_ledger() override {
        return true; // Stub
    }
};

static auto reg = []() {
    Registry<IAuditLogger>::instance().add("default", []() {
        return std::make_unique<AuditLoggerImpl>();
    });
    return true;
}();

} // namespace fo::core
