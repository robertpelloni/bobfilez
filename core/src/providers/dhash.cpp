#include "fo/providers/dhash.hpp"

#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace fo::providers {

    DHash::DHash() = default;
    DHash::~DHash() = default;

    std::string DHash::name() const {
        return "dhash";
    }

    std::string DHash::fast64(const std::filesystem::path& p) {
        std::ifstream file(p, std::ios::binary);
        if (!file) {
            return "";
        }

        // Read the file into a buffer
        file.seekg(0, std::ios::end);
        std::vector<char> buffer(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), buffer.size());

        // Load the image
        int width, height, channels;
        unsigned char* data = stbi_load_from_memory(
            reinterpret_cast<const stbi_uc*>(buffer.data()),
            buffer.size(),
            &width,
            &height,
            &channels,
            1 // Force grayscale
        );

        if (!data) {
            return "";
        }

        // Downscale to 9x8
        std::vector<unsigned char> scaled(9 * 8);
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 9; ++x) {
                int orig_x = x * width / 9;
                int orig_y = y * height / 8;
                scaled[y * 9 + x] = data[orig_y * width + orig_x];
            }
        }

        stbi_image_free(data);

        // Compute the hash. Bit convention must match the OpenCV
        // perceptual hasher (DHashPerceptualHasher): bit set when the
        // left pixel is brighter. The old '<' produced the bitwise
        // complement, so hashes stored via `hash --hasher=dhash` never
        // matched `similar` targets (distance ~64 for identical files).
        uint64_t hash = 0;
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                if (scaled[y * 9 + x] > scaled[y * 9 + x + 1]) {
                    hash |= (1ULL << (y * 8 + x));
                }
            }
        }

        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << hash;
        return ss.str();
    }

} // namespace fo::providers
