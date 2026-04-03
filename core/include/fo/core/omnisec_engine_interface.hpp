#pragma once
/// @file omnisec_engine_interface.hpp
/// @brief Cybersecurity, Reverse Engineering & Cryptanalysis Engine for bobfilez.
///
/// OmniSec leverages powerful submodules to analyze files at the lowest level:
///   - radare2 (r2) for binary disassembly and reverse engineering.
///   - hashcat for GPU-accelerated password recovery on locked archives.
///   - ssdeep for fuzzy hashing and malware similarity detection.

#include <string>
#include <vector>
#include <filesystem>
#include <memory>

namespace fo::core {

struct BinaryFunction {
    std::string name;
    std::string vaddr;
    std::string opcodes;
    size_t size_bytes;
};

struct MalwareReport {
    std::string fuzzy_hash; // ssdeep hash
    double threat_score;    // 0.0 to 1.0
    std::vector<std::string> suspicious_imports;
};

struct CrackResult {
    bool success;
    std::string recovered_password;
    double time_taken_sec;
    std::string hash_type;
};

class IOmniSecEngine {
public:
    virtual ~IOmniSecEngine() = default;

    /// Disassemble an executable file using radare2
    virtual std::vector<BinaryFunction> disassemble_binary(const std::filesystem::path& file) = 0;

    /// Analyze a file for malware signatures and fuzzy hashes (ssdeep)
    virtual MalwareReport analyze_threats(const std::filesystem::path& file) = 0;

    /// Attempt to recover a lost password for a ZIP/7z/RAR archive using hashcat
    virtual CrackResult crack_archive(const std::filesystem::path& archive, const std::filesystem::path& wordlist) = 0;
};

} // namespace fo::core
