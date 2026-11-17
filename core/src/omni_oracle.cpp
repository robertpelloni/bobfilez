/// @file omni_oracle.cpp
/// @brief Implementation of the Local LLM RAG Copilot.

#include "fo/core/omni_oracle_interface.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/document_embedder_interface.hpp"
#include <iostream>
#include <chrono>

namespace fo::core {

class OmniOracleImpl : public IOmniOracle {
    bool is_ready_ = false;

public:
    OmniOracleImpl() {
        // Pre-load state logic if needed
    }

    bool initialize(const std::filesystem::path& model_path) override {
        std::cout << "[OmniOracle] Loading Local LLM into VRAM: " << model_path << "\n";
        is_ready_ = true;
        return true;
    }

    bool is_ready() const override { return is_ready_; }

    OracleResponse ask(const std::string& query) override {
        std::cout << "[OmniOracle] Received Query: " << query << "\n";
        
        auto t0 = std::chrono::steady_clock::now();

        // RAG Workflow Simulation:
        // 1. Send query to DocumentEmbedder (BERT)
        // 2. Fetch Top-K nearest chunks from SQLite 'file_embeddings'
        // 3. Inject chunks into LLM prompt
        // 4. Generate answer
        
        OracleResponse res;

        // No local model or embedding index is wired up yet (the RAG
        // steps above are the design, not an implementation). Answer
        // honestly instead of fabricating file citations and counts,
        // which the previous mock did for every query.
        res.answer_text =
            "OmniOracle is not connected to a local model or document "
            "index yet, so I can't answer questions about your files. "
            "Your question was: \"" + query + "\". "
            "Use Search for filename/content lookup today; Oracle Q&A "
            "arrives when the embedder + local LLM are integrated.";

        auto t1 = std::chrono::steady_clock::now();
        res.generation_time_sec = std::chrono::duration<double>(t1 - t0).count();

        return res;
    }

    OracleResponse summarize(const std::filesystem::path& file) override {
        std::cout << "[OmniOracle] Summarizing file: " << file << "\n";
        OracleResponse res;
        // Same honesty rule as ask(): no model is loaded, so do not
        // invent a summary of a file that was never read.
        res.answer_text =
            "OmniOracle is not connected to a local model yet, so I "
            "can't summarize " + file.string() + ".";
        return res;
    }
};

static bool register_omni_oracle_guard = []() {
    Registry<IOmniOracle>::instance().add("default", []() {
        return std::make_unique<OmniOracleImpl>();
    });
    return true;
}();

void register_omni_oracle() { (void)register_omni_oracle_guard; }

} // namespace fo::core
