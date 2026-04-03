#pragma once
/// @file omniterminal_interface.hpp
/// @brief AI-Powered Terminal Emulator for bobfilez OmniShell.
///
/// Integrates a full PTY-based terminal (cmd/PowerShell/bash/zsh) with
/// OmniOracle AI command suggestions and context-aware autocomplete.

#include <string>
#include <vector>
#include <functional>

namespace fo::core {

struct CommandSuggestion {
    std::string command;
    std::string description;
    double confidence;
};

class IOmniTerminalEngine {
public:
    virtual ~IOmniTerminalEngine() = default;

    /// Start a new shell session (cmd.exe, powershell, bash)
    virtual bool start_session(const std::string& shell = "auto") = 0;

    /// Send input to the active PTY
    virtual void send_input(const std::string& input) = 0;

    /// Get AI-powered command suggestions based on current context
    virtual std::vector<CommandSuggestion> get_ai_suggestions(const std::string& partial_input, const std::string& cwd) = 0;

    /// Set output callback for rendering in QML
    virtual void set_output_callback(std::function<void(const std::string&)> cb) = 0;
};

} // namespace fo::core
