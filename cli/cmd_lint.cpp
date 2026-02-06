#include "fo/core/registry.hpp"
#include "fo/core/lint_interface.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

void lint_command(const std::vector<std::string>& args) {
    auto linter = fo::core::Registry<fo::core::ILinter>::instance().create("std");
    if (!linter) {
        std::cerr << "Error: Standard linter not found.\n";
        return;
    }

    std::vector<std::filesystem::path> paths;
    for (const auto& arg : args) {
        if (arg.rfind("--", 0) != 0) { // Not an option
             paths.push_back(arg);
        }
    }

    if (paths.empty()) {
        std::cerr << "Usage: fo_cli lint <path> [options]\n";
        return;
    }

    auto results = linter->lint(paths);

    std::cout << "Lint Results:\n";
    for (const auto& res : results) {
        std::string type_str;
        switch (res.type) {
            case fo::core::LintType::EmptyFile: type_str = "EMPTY FILE"; break;
            case fo::core::LintType::EmptyDirectory: type_str = "EMPTY DIR"; break;
            case fo::core::LintType::BrokenSymlink: type_str = "BROKEN LINK"; break;
            case fo::core::LintType::TemporaryFile: type_str = "TEMP FILE"; break;
        }
        std::cout << "[" << type_str << "] " << res.path.string() << " (" << res.details << ")\n";
    }
}
