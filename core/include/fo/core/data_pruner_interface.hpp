#pragma once
/// @file data_pruner_interface.hpp
/// @brief Autonomous "Digital Rot" Detection Engine for bobfilez.
///
/// Identifies files that are likely safe to delete or archive:
///   - Obsolete Installers (older versions of Chrome, Node, etc.)
///   - Large Temporary Files (.tmp, .log, .cache)
///   - Blurry/Corrupt Photos (via basic image analysis)
///   - "Forgotten" Downloads (large files not accessed in >1 year)
///   - Incomplete Downloads (.part, .crdownload)
///
/// Provides a "Confidence Score" for each suggestion.

#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

namespace fo::core {

struct PruningSuggestion {
    std::filesystem::path path;
    std::string reason;      // e.g. "Obsolete Installer", "Digital Rot"
    double confidence;       // 0.0 - 1.0
    uintmax_t size;
    bool safe_to_auto_delete = false;
};

class IDataPruner {
public:
    virtual ~IDataPruner() = default;

    /// Scan a directory for potential pruning targets
    virtual std::vector<PruningSuggestion> analyze(const std::filesystem::path& root) = 0;

    /// Filter suggestions by minimum confidence
    virtual void set_min_confidence(double score) = 0;

    /// Register a custom pruning rule (regex based)
    virtual void add_custom_rule(const std::string& name, const std::string& pattern, double weight) = 0;
};

} // namespace fo::core
