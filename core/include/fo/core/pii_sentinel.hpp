#pragma once
/// @file pii_sentinel.hpp
/// @brief Personally Identifiable Information (PII) detection for bobfilez.
///
/// Scans file contents for sensitive patterns:
///   - Social Security Numbers (SSN)
///   - Credit Card Numbers (PAN)
///   - API Keys (AWS, Google, Stripe)
///   - Private Keys (RSA, SSH)
///   - Passwords / Secrets
///
/// Integrated into the RuleEngine to automatically flag or move sensitive 
/// documents to an encrypted "Vault" folder.

#include <string>
#include <vector>
#include <regex>
#include <filesystem>

namespace fo::core {

struct PiiMatch {
    std::string type;       // e.g. "SSN", "CreditCard"
    std::string snippet;    // Context around match (masked)
    int64_t line_number;
    double confidence;      // 0.0 - 1.0
};

class PIISentinel {
public:
    /// Scan a file for PII patterns
    std::vector<PiiMatch> scan_file(const std::filesystem::path& path);

    /// Scan a raw string for PII patterns
    std::vector<PiiMatch> scan_string(const std::string& content);

    /// Mask sensitive parts of a string for safe display (e.g. 123-XX-XXXX)
    static std::string mask(const std::string& pii, const std::string& type);

private:
    struct PiiPattern {
        std::string name;
        std::regex regex;
        double weight;
    };
    std::vector<PiiPattern> patterns_ = {
        {"SSN", std::regex(R"(\b\d{3}-\d{2}-\d{4}\b)"), 1.0},
        {"CreditCard", std::regex(R"(\b(?:\d[ -]*?){13,16}\b)"), 0.9},
        {"AWS_Key", std::regex(R"(AKIA[0-9A-Z]{16})"), 1.0},
        {"Private_Key", std::regex(R"(-----BEGIN [A-Z ]+ PRIVATE KEY-----)"), 1.0},
        {"Email", std::regex(R"(\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Z|a-z]{2,}\b)"), 0.5}
    };
};

} // namespace fo::core
