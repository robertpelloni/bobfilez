#pragma once

#include <string>
#include <filesystem>
#include <optional>
#include <vector>

namespace fo::core {

struct OCRResult {
    std::string text;
    float confidence = 0.0f; // 0..1
    std::string lang;
};

struct OCRBoundingBox {
    int x, y, width, height;
    std::string text;
    float confidence = 0.0f;
};

class IOCRProvider {
public:
    virtual ~IOCRProvider() = default;
    virtual std::string name() const = 0;
    
    // Simple full-page OCR
    virtual std::optional<OCRResult> recognize(const std::filesystem::path& image_path, const std::string& lang = "eng") = 0;

    // Why this provider cannot run right now (e.g. its language
    // data cannot be loaded), or nullopt when ready. Callers that
    // report success/failure to users must check this: recognize()
    // alone cannot distinguish "engine unavailable" from "no text".
    virtual std::optional<std::string> unavailable_reason(const std::string& lang = "eng") {
        (void)lang;
        return std::nullopt;
    }

    // Advanced: OCR with word-level bounding boxes (optional)
    virtual std::vector<OCRBoundingBox> recognize_detailed(const std::filesystem::path& image_path, const std::string& lang = "eng") {
        (void)image_path; (void)lang;
        return {}; // Default: not implemented
    }
};

} // namespace fo::core
