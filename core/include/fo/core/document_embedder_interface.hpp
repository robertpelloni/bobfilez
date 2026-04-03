#pragma once
/// @file document_embedder_interface.hpp
/// @brief Semantic document embedding engine for bobfilez.
///
/// Uses small Transformer models (BERT/MiniLM) via ONNX Runtime to index
/// document contents (TXT, PDF, DOCX, MD) into high-dimensional vectors.
/// Enables natural language search inside files:
///   "Find documents discussing the quarterly budget"

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace fo::core {

struct DocumentEmbedding {
    std::vector<float> values;
};

class IDocumentEmbedder {
public:
    virtual ~IDocumentEmbedder() = default;

    /// Initialize the Transformer model (ONNX)
    virtual bool initialize(const std::filesystem::path& model_path,
                            const std::filesystem::path& vocab_path) = 0;

    /// Compute embedding for a block of text
    virtual std::optional<DocumentEmbedding> embed_text(const std::string& text) = 0;

    /// Extract and embed contents of a file (PDF, DOCX, MD)
    virtual std::optional<DocumentEmbedding> embed_file(const std::filesystem::path& path) = 0;

    virtual bool is_ready() const = 0;
};

} // namespace fo::core
