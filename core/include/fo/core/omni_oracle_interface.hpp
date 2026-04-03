#pragma once
/// @file omni_oracle_interface.hpp
/// @brief Local LLM Copilot & RAG (Retrieval-Augmented Generation) Engine.
///
/// OmniOracle is the conversational interface to the user's filesystem.
/// It uses the DocumentEmbedder to find relevant file contents, injects them
/// into a local LLM prompt, and generates intelligent, context-aware answers
/// with file citations and actionable UI triggers.

#include <string>
#include <vector>
#include <filesystem>
#include <memory>

namespace fo::core {

struct OracleCitation {
    std::filesystem::path file_path;
    std::string snippet;
    double relevance_score;
};

struct OracleAction {
    std::string action_id; // e.g., "omniflow:execute", "fileops:move"
    std::string label;     // e.g., "Run Auto-Vault Automation"
    std::string payload;   // JSON string of parameters
};

struct OracleResponse {
    std::string answer_text;
    std::vector<OracleCitation> citations;
    std::vector<OracleAction> suggested_actions;
    double generation_time_sec;
};

class IOmniOracle {
public:
    virtual ~IOmniOracle() = default;

    /// Initialize the local LLM (e.g., Llama.cpp, ONNX Phi-3/Mistral)
    virtual bool initialize(const std::filesystem::path& model_path) = 0;

    /// Ask the Oracle a question about the filesystem or specific documents
    virtual OracleResponse ask(const std::string& query) = 0;

    /// Ask the Oracle to summarize a specific file
    virtual OracleResponse summarize(const std::filesystem::path& file) = 0;

    /// True if the LLM is loaded in memory and ready
    virtual bool is_ready() const = 0;
};

} // namespace fo::core
