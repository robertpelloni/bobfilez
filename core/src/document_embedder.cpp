/// @file document_embedder.cpp
/// @brief Implementation of Transformer-based document embeddings.

#include "fo/core/document_embedder_interface.hpp"
#include "fo/core/registry.hpp"
#include <iostream>

namespace fo::core {

class OnnxDocumentEmbedder : public IDocumentEmbedder {
    bool is_ready_ = false;

public:
    bool initialize(const std::filesystem::path& model_path, const std::filesystem::path& vocab_path) override {
        std::cout << "[DocEmbed] Initializing Transformer model: " << model_path << "\n";
        is_ready_ = true;
        return true;
    }

    std::optional<DocumentEmbedding> embed_text(const std::string& text) override {
        if (!is_ready_) return std::nullopt;
        // Placeholder for ONNX inference logic (same pattern as clip_search_engine)
        return DocumentEmbedding{{0.1f, -0.5f, 0.9f}}; 
    }

    std::optional<DocumentEmbedding> embed_file(const std::filesystem::path& path) override {
        // 1. Extract text via Pandoc / Poppler
        // 2. Compute embedding
        return embed_text("Extracted content placeholder");
    }

    bool is_ready() const override { return is_ready_; }
};

static auto reg = []() {
    Registry<IDocumentEmbedder>::instance().add("onnx_bert", []() {
        return std::make_unique<OnnxDocumentEmbedder>();
    });
    return true;
}();

} // namespace fo::core
