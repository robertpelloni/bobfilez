#pragma once
/// @file photo_catalog_interface.hpp
/// @brief Pro-grade Photo Library and Catalog system for bobfilez.
///
/// Provides parity with Adobe Lightroom Classic and Apple Photos:
///   - Non-destructive edit history (sidecar or DB based)
///   - Facial Recognition & People grouping
///   - GPS/Geotagging & Map visualization
///   - AI Scene/Object tagging (Google Photos style)
///   - RAW development pipeline (LibRaw/Exiv2)

#include <string>
#include <vector>
#include <filesystem>
#include <map>

namespace fo::core {

struct FaceMatch {
    std::string person_id;
    std::string name;
    double confidence;
    float box_x, box_y, box_w, box_h;
};

struct PhotoMetadata {
    std::filesystem::path path;
    int width, height;
    std::string camera_make, camera_model;
    double latitude, longitude;
    std::vector<std::string> ai_tags;
    std::vector<FaceMatch> faces;
    bool is_raw = false;
    std::string color_palette; // Hex codes
};

struct PhotoAdjustment {
    double exposure = 0.0;
    double temperature = 0.0; // Kelvin-relative
    double tint = 0.0;
    double highlights = 0.0;
    double shadows = 0.0;
    double whites = 0.0;
    double blacks = 0.0;
    double clarity = 0.0;
    double vibrance = 0.0;
    double saturation = 0.0;
    // Tone curve points
    std::vector<std::pair<double, double>> tone_curve;
};

class IPhotoCatalog {
public:
    virtual ~IPhotoCatalog() = default;

    /// Scan and index a photo directory
    virtual void import_photos(const std::filesystem::path& root) = 0;

    /// Search for photos by AI tag, person, or location
    virtual std::vector<PhotoMetadata> query(const std::string& filter) = 0;

    /// Apply non-destructive adjustments
    virtual void save_adjustments(const std::filesystem::path& photo, const PhotoAdjustment& adj) = 0;

    /// Load non-destructive adjustments
    virtual PhotoAdjustment load_adjustments(const std::filesystem::path& photo) = 0;

    /// Detect faces in a photo
    virtual std::vector<FaceMatch> detect_faces(const std::filesystem::path& photo) = 0;
};

} // namespace fo::core
