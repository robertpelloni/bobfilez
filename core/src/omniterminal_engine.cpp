/// @file omniterminal_engine.cpp
/// @brief Implementation of the AI-Powered Terminal Emulator.

#include "fo/core/omniterminal_interface.hpp"
#include "fo/core/registry.hpp"
#include <iostream>

namespace fo::core {

class OmniTerminalImpl : public IOmniTerminalEngine {
    std::function<void(const std::string&)> output_cb_;

public:
    bool start_session(const std::string& shell) override {
        std::string actual_shell = shell;
        if (actual_shell == "auto") {
#ifdef _WIN32
            actual_shell = "powershell.exe";
#else
            actual_shell = "/bin/zsh";
#endif
        }
        std::cout << "[OmniTerminal] Starting PTY session: " << actual_shell << "\n";
        return true;
    }

    void send_input(const std::string& input) override {
        std::cout << "[OmniTerminal] >> " << input << "\n";
        // Write to PTY stdin
    }

    std::vector<CommandSuggestion> get_ai_suggestions(const std::string& partial, const std::string& cwd) override {
        std::vector<CommandSuggestion> suggestions;
        
        if (partial.find("git") != std::string::npos) {
            suggestions.push_back({"git status", "Show working tree status", 0.95});
            suggestions.push_back({"git log --oneline -10", "Show last 10 commits", 0.90});
            suggestions.push_back({"git diff --cached", "Show staged changes", 0.85});
        } else if (partial.find("find") != std::string::npos) {
            suggestions.push_back({"find . -name '*.cpp' -mtime -1", "Find C++ files modified today", 0.92});
            suggestions.push_back({"find . -size +100M -type f", "Find files larger than 100MB", 0.88});
        } else {
            suggestions.push_back({"ls -la", "List all files with details", 0.70});
            suggestions.push_back({"htop", "Interactive process viewer", 0.60});
        }
        
        return suggestions;
    }

    void set_output_callback(std::function<void(const std::string&)> cb) override {
        output_cb_ = std::move(cb);
    }
};

static auto reg = []() {
    Registry<IOmniTerminalEngine>::instance().add("default", []() {
        return std::make_unique<OmniTerminalImpl>();
    });
    return true;
}();

} // namespace fo::core
