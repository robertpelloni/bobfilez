#pragma once

#include <filesystem>
#include <string>
#include <optional>
#include <vector>

namespace fo::core {

// Thumbnail generator for image files
class ThumbnailGenerator {
public:
    // Maximum thumbnail dimensions
    static constexpr int DEFAULT_WIDTH = 150;
    static constexpr int DEFAULT_HEIGHT = 150;

    // Supported image extensions
    static bool is_image_file(const std::filesystem::path& path);

    // Generate a thumbnail and return as base64-encoded JPEG
    // Returns std::nullopt if the file is not an image or generation fails
    static std::optional<std::string> generate_base64(
        const std::filesystem::path& path,
        int max_width = DEFAULT_WIDTH,
        int max_height = DEFAULT_HEIGHT);

    // Generate a thumbnail and save to a file
    static bool generate_to_file(
        const std::filesystem::path& source,
        const std::filesystem::path& dest,
        int max_width = DEFAULT_WIDTH,
        int max_height = DEFAULT_HEIGHT);

    // Base64 encoding helper (public for testability)
    static std::string base64_encode(const std::vector<unsigned char>& data);
};

} // namespace fo::core

