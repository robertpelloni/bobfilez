#pragma once

#include "types.hpp"
#include <string>
#include <vector>
#include <optional>

namespace fo::core {

enum class LintType {
    EmptyFile,
    EmptyDirectory,
    BrokenSymlink,
    TemporaryFile,
    HiddenFile,         // Files starting with '.' or '$' that may be system artifacts
    DuplicateName,      // Same filename exists in multiple directories
    LargeFile,          // Files exceeding a configurable size threshold
    DeepNesting,        // Directories nested beyond a threshold
};

struct LintResult {
    std::filesystem::path path;
    LintType type;
    std::string details;
};

class ILinter {
public:
    virtual ~ILinter() = default;
    virtual std::string name() const = 0;
    virtual std::vector<LintResult> lint(const std::vector<std::filesystem::path>& roots) = 0;
};

} // namespace fo::core
