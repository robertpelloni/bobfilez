#pragma once
/// @file semantic_tagging_interface.hpp
/// @brief AI-driven automatic tagging system for bobfilez.
///
/// Uses computer vision (MobileNet/CLIP) and NLP (BERT) to analyze file
/// contents and assign semantic tags automatically.
/// Examples:
///   - Image of a dog -> "Animals", "Pets", "Dog"
///   - PDF of a bill -> "Finance", "Invoice", "2024"
///   - Video of a sunset -> "Nature", "Sunset", "Video"

#include <string>
#include <vector>
#include <filesystem>
#include <map>

namespace fo::core {

struct SemanticTag {
    std::string name;
    double confidence;
    std::string source_model; // "MobileNet", "CLIP", "BERT"
};

class ISemanticTaggingEngine {
public:
    virtual ~ISemanticTaggingEngine() = default;

    /// Analyze a file and return a list of suggested tags
    virtual std::vector<SemanticTag> analyze_file(const std::filesystem::path& path) = 0;

    /// Bulk tag a list of files
    virtual std::map<std::filesystem::path, std::vector<SemanticTag>> bulk_analyze(const std::vector<std::filesystem::path>& files) = 0;

    /// Train/Fine-tune (Stub) - Learn user preferences for tags
    virtual void learn_from_user(const std::filesystem::path& path, const std::string& correct_tag) = 0;
};

} // namespace fo::core
