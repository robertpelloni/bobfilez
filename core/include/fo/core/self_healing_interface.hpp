#pragma once
/// @file self_healing_interface.hpp
/// @brief Proactive bit-rot detection and silent corruption recovery for bobfilez.
///
/// Uses the Nexus scheduler to perform background "Scrubbing" of files.
/// Features:
///   - Checksum Verification (XXH64/SHA256) against DB records
///   - Silent Bit-rot detection
///   - Auto-recovery from P2P Swarm or Cloud mirrors
///   - Health reporting

#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

namespace fo::core {

struct CorruptionReport {
    std::filesystem::path path;
    std::string expected_hash;
    std::string actual_hash;
    std::chrono::system_clock::time_point detected_at;
    bool can_recover = false;
    std::string recovery_source; // "S3", "Swarm:Laptop-X1", "Local-Backup"
};

class ISelfHealingEngine {
public:
    virtual ~ISelfHealingEngine() = default;

    /// Start a background scrub of a directory
    virtual void start_scrub(const std::filesystem::path& root) = 0;

    /// Stop an active scrub
    virtual void stop_scrub() = 0;

    /// Get list of detected corruptions
    virtual std::vector<CorruptionReport> get_reports() = 0;

    /// Attempt to heal a file from available redundant sources
    virtual bool heal_file(const std::filesystem::path& path) = 0;

    /// Set scrub schedule (e.g. Weekly)
    virtual void set_schedule_days(int days) = 0;

    /// Register a known-good hash for a file (baseline)
    virtual void register_baseline(const std::filesystem::path& path, const std::string& hash) = 0;

    /// Verify a single file against its baseline hash
    virtual CorruptionReport verify_file(const std::filesystem::path& path, const std::string& known_hash) = 0;

    /// Scrub a directory: verify all files with registered baselines
    virtual int scrub_directory(const std::filesystem::path& root) = 0;
};

} // namespace fo::core
