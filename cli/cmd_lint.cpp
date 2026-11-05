#include "fo/core/registry.hpp"
#include "fo/core/lint_interface.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

namespace {

std::string json_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
        }
    }
    return out;
}

std::string lint_type_name(fo::core::LintType t) {
    switch (t) {
        case fo::core::LintType::EmptyFile: return "EMPTY FILE";
        case fo::core::LintType::EmptyDirectory: return "EMPTY DIR";
        case fo::core::LintType::BrokenSymlink: return "BROKEN LINK";
        case fo::core::LintType::TemporaryFile: return "TEMP FILE";
        case fo::core::LintType::HiddenFile: return "HIDDEN FILE";
        case fo::core::LintType::DuplicateName: return "DUPLICATE NAME";
        case fo::core::LintType::LargeFile: return "LARGE FILE";
        case fo::core::LintType::DeepNesting: return "DEEP NESTING";
    }
    return "UNKNOWN";
}

} // namespace

void lint_command(const std::vector<std::string>& paths, bool json_output) {
    auto linter = fo::core::Registry<fo::core::ILinter>::instance().create("std");
    if (!linter) {
        std::cerr << "Error: Standard linter not found.\n";
        return;
    }

    std::vector<std::filesystem::path> fs_paths(paths.begin(), paths.end());
    if (fs_paths.empty()) {
        std::cerr << "Usage: fo_cli lint <path> [options]\n";
        return;
    }

    auto results = linter->lint(fs_paths);

    if (json_output) {
        std::cout << "[";
        bool first = true;
        for (const auto& res : results) {
            if (!first) std::cout << ",";
            first = false;
            std::cout << "{\"path\":\"" << json_escape(res.path.string())
                      << "\",\"type\":\"" << lint_type_name(res.type)
                      << "\",\"details\":\"" << json_escape(res.details) << "\"}";
        }
        std::cout << "]";
        return;
    }

    std::cout << "Lint Results:\n";
    for (const auto& res : results) {
        std::cout << "[" << lint_type_name(res.type) << "] " << res.path.string() << " (" << res.details << ")\n";
    }
}