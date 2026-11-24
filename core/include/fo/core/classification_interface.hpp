#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace fo::core {

struct ClassificationResult {
    std::string label;
    float confidence;
};

class IImageClassifier {
public:
    virtual ~IImageClassifier() = default;
    virtual std::string name() const = 0;
    
    // Classify an image, returning top-k results
    virtual std::vector<ClassificationResult> classify(
        const std::filesystem::path& image_path,
        int top_k = 3) = 0;

    // Why this classifier cannot run right now (e.g. its model file
    // is missing), or nullopt when ready. Callers that report
    // success/failure to users must check this: classify() alone
    // cannot distinguish "model unavailable" from "no labels".
    virtual std::optional<std::string> unavailable_reason() { return std::nullopt; }
};

} // namespace fo::core
