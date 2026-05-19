#pragma once
/// @file clip_search_interface.hpp
/// @brief Vector-Semantic Search engine interface for bobfilez.
///
/// Implements natural language image search using OpenAI's CLIP model running locally.
/// Enables querying files like "a dog in the grass" and returning matching images.
/// 
/// Supported models:
///   - ViT-B/32 (Vision Transformer) ONNX export
///   - RN50 (ResNet-50) ONNX export
/// 
/// The engine is completely offline, ensuring user privacy.

#include <string>
#include <cmath>
#include <vector>
#include <filesystem>
#include <cstdint>
#include <optional>

namespace fo::core {

/// A 512-dimensional float vector representing a CLIP embedding
struct ClipEmbedding {
    std::vector<float> values;

    /// Cosine similarity (dot product of L2-normalized vectors)
    double similarity(const ClipEmbedding& other) const {
        if (values.size() != other.values.size() || values.empty()) return 0.0;
        double dot = 0.0, norm_a = 0.0, norm_b = 0.0;
        for (size_t i = 0; i < values.size(); ++i) {
            dot += values[i] * other.values[i];
            norm_a += values[i] * values[i];
            norm_b += other.values[i] * other.values[i];
        }
        if (norm_a == 0.0 || norm_b == 0.0) return 0.0;
        return dot / (std::sqrt(norm_a) * std::sqrt(norm_b));
    }
};

/// A single result from a semantic search
struct SemanticSearchResult {
    std::filesystem::path path;
    double score = 0.0;       // Cosine similarity [0.0, 1.0]
    int64_t file_size = 0;
};

/// Abstract interface for the Vector-Semantic Search Engine
class IClipSearchEngine {
public:
    virtual ~IClipSearchEngine() = default;

    /// Initialize the ONNX models (Vision + Text)
    /// Returns true if successfully loaded.
    virtual bool initialize(const std::filesystem::path& vision_model_path,
                            const std::filesystem::path& text_model_path,
                            const std::filesystem::path& bpe_vocab_path) = 0;

    /// Compute the embedding for an image
    virtual std::optional<ClipEmbedding> embed_image(const std::filesystem::path& image_path) = 0;

    /// Compute the embedding for a text query
    virtual std::optional<ClipEmbedding> embed_text(const std::string& query) = 0;

    /// Search a pre-computed database of embeddings using a text query.
    /// @param query The natural language text (e.g. "a blue car")
    /// @param top_k Max number of results to return
    /// @param threshold Minimum similarity score (e.g. 0.22)
    virtual std::vector<SemanticSearchResult> search(const std::string& query, 
                                                     int top_k = 50, 
                                                     double threshold = 0.20) = 0;

    /// True if models are loaded and engine is ready
    virtual bool is_ready() const = 0;
};

} // namespace fo::core
