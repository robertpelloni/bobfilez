#pragma once
/// @file neural_bridge_interface.hpp
/// @brief Unified AI/ML model coordinator for bobfilez.
///
/// Orchestrates all local neural networks:
///   - CLIP (Vector-Semantic Search)
///   - Tesseract (OCR)
///   - MobileNet/SqueezeNet (Image Classification)
///   - Custom PII detection models
///
/// Handles model loading, warm-up, GPU/CPU fallback, and memory management.

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <variant>
#include "fo/core/clip_search_interface.hpp"
#include "fo/core/classification_interface.hpp"
#include "fo/core/ocr_interface.hpp"
#include "fo/core/document_embedder_interface.hpp"

namespace fo::core {

struct ModelStatus {
    std::string name;
    bool loaded;
    size_t memory_usage_bytes;
    std::string device; // "CPU", "CUDA", "DirectML"
};

class INeuralBridge {
public:
    virtual ~INeuralBridge() = default;

    /// Load a specific model suite
    virtual bool load_suite(const std::string& suite_name) = 0;

    /// Get the specific engine instances
    virtual IClipSearchEngine* clip() = 0;
    virtual IImageClassifier* classifier() = 0;
    virtual IOCRProvider* ocr() = 0;
    virtual IDocumentEmbedder* bert() = 0;

    /// Performance & Memory
    virtual std::vector<ModelStatus> get_status() = 0;
    virtual void unload_idle_models() = 0;
};

} // namespace fo::core
