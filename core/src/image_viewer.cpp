/// @file image_viewer.cpp
/// @brief Implementation of ImageViewer backend: thumbnail caching, histogram, color adjustments.
///
/// Uses stb_image for lightweight decoding when OpenCV/Qt are unavailable.
/// Histogram computation and color adjustments operate on raw pixel buffers.
/// ThumbnailCache uses a simple LRU strategy with configurable max entries.

#include "fo/core/image_viewer_interface.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cmath>
#include <mutex>
#include <list>
#include <unordered_map>

namespace fo::core {

// ─── Thumbnail Cache (LRU) ─────────────────────────────────────────────────
class ThumbnailCacheImpl {
    struct CacheEntry {
        std::filesystem::path path;
        std::vector<uint8_t> thumbnail_data;  // RGBA pixels
        int width = 0, height = 0;
    };

    size_t max_entries_;
    std::mutex mutex_;
    std::list<CacheEntry> entries_;  // Front = most recently used
    std::unordered_map<std::string, std::list<CacheEntry>::iterator> lookup_;

public:
    explicit ThumbnailCacheImpl(size_t max_entries = 500) : max_entries_(max_entries) {}

    /// Check if a thumbnail is cached
    bool has(const std::filesystem::path& path) {
        std::lock_guard<std::mutex> lock(mutex_);
        return lookup_.find(path.string()) != lookup_.end();
    }

    /// Get cached thumbnail data (moves to front of LRU list)
    bool get(const std::filesystem::path& path, std::vector<uint8_t>& out, int& w, int& h) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = lookup_.find(path.string());
        if (it == lookup_.end()) return false;

        // Move to front (most recently used)
        entries_.splice(entries_.begin(), entries_, it->second);
        out = it->second->thumbnail_data;
        w = it->second->width;
        h = it->second->height;
        return true;
    }

    /// Store a thumbnail in the cache
    void put(const std::filesystem::path& path, const std::vector<uint8_t>& data, int w, int h) {
        std::lock_guard<std::mutex> lock(mutex_);

        // If already exists, update
        auto it = lookup_.find(path.string());
        if (it != lookup_.end()) {
            it->second->thumbnail_data = data;
            it->second->width = w;
            it->second->height = h;
            entries_.splice(entries_.begin(), entries_, it->second);
            return;
        }

        // Evict LRU if at capacity
        while (entries_.size() >= max_entries_) {
            auto& back = entries_.back();
            lookup_.erase(back.path.string());
            entries_.pop_back();
        }

        // Insert new entry at front
        entries_.push_front({path, data, w, h});
        lookup_[path.string()] = entries_.begin();
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        entries_.clear();
        lookup_.clear();
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mutex_);
        return entries_.size();
    }
};


// ─── Histogram Computation ──────────────────────────────────────────────────

/// Compute RGB histogram from raw RGBA pixel data
/// Returns a struct with 3 channels x 256 bins each
struct RGBHistogram {
    int r[256] = {};
    int g[256] = {};
    int b[256] = {};
    int luminance[256] = {};
};

static RGBHistogram compute_histogram(const uint8_t* pixels, int width, int height, int channels) {
    RGBHistogram hist = {};
    int total_pixels = width * height;

    for (int i = 0; i < total_pixels; ++i) {
        int offset = i * channels;
        uint8_t r = pixels[offset];
        uint8_t g = pixels[offset + 1];
        uint8_t b = (channels >= 3) ? pixels[offset + 2] : g;

        hist.r[r]++;
        hist.g[g]++;
        hist.b[b]++;

        // Perceived luminance: 0.299R + 0.587G + 0.114B
        uint8_t lum = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);
        hist.luminance[lum]++;
    }

    return hist;
}


// ─── Color Adjustments ─────────────────────────────────────────────────────

/// Apply brightness, contrast, saturation, and gamma adjustments in-place
/// on an RGBA pixel buffer.
///
/// @param pixels     Raw RGBA (or RGB) data
/// @param count      Number of pixels
/// @param channels   3 (RGB) or 4 (RGBA)
/// @param brightness [-100, 100] additive offset
/// @param contrast   [-100, 100] multiplicative scaling
/// @param saturation [-100, 100] color intensity
/// @param gamma      [0.1, 5.0] gamma correction exponent
static void apply_color_adjustments(uint8_t* pixels, int count, int channels,
                                     int brightness, int contrast, int saturation, double gamma) {
    // Precompute contrast factor: f = (259*(contrast+255)) / (255*(259-contrast))
    double c_factor = (259.0 * (contrast + 255.0)) / (255.0 * (259.0 - contrast));

    // Precompute gamma LUT
    uint8_t gamma_lut[256];
    for (int i = 0; i < 256; ++i) {
        double normalized = i / 255.0;
        double corrected = std::pow(normalized, 1.0 / gamma);
        gamma_lut[i] = static_cast<uint8_t>(std::clamp(corrected * 255.0, 0.0, 255.0));
    }

    double sat_factor = 1.0 + saturation / 100.0;

    for (int i = 0; i < count; ++i) {
        int off = i * channels;
        double r = pixels[off];
        double g = pixels[off + 1];
        double b = (channels >= 3) ? pixels[off + 2] : g;

        // 1. Brightness (additive)
        r += brightness;
        g += brightness;
        b += brightness;

        // 2. Contrast (multiplicative around midpoint 128)
        r = c_factor * (r - 128.0) + 128.0;
        g = c_factor * (g - 128.0) + 128.0;
        b = c_factor * (b - 128.0) + 128.0;

        // 3. Saturation (mix with luminance)
        double lum = 0.299 * r + 0.587 * g + 0.114 * b;
        r = lum + sat_factor * (r - lum);
        g = lum + sat_factor * (g - lum);
        b = lum + sat_factor * (b - lum);

        // Clamp to [0, 255]
        uint8_t ri = static_cast<uint8_t>(std::clamp(r, 0.0, 255.0));
        uint8_t gi = static_cast<uint8_t>(std::clamp(g, 0.0, 255.0));
        uint8_t bi = static_cast<uint8_t>(std::clamp(b, 0.0, 255.0));

        // 4. Gamma correction (via LUT)
        pixels[off]     = gamma_lut[ri];
        pixels[off + 1] = gamma_lut[gi];
        if (channels >= 3) pixels[off + 2] = gamma_lut[bi];
        // Alpha channel (if present) is left untouched
    }
}


// ─── EXIF Reader (minimal) ─────────────────────────────────────────────────

/// Extract basic EXIF tags from JPEG files by parsing the APP1 marker.
/// Returns a map of tag names to string values.
/// This is a simplified parser; for full EXIF support, use libexiv2.
static std::unordered_map<std::string, std::string> read_basic_exif(const std::filesystem::path& path) {
    std::unordered_map<std::string, std::string> tags;

    std::ifstream f(path, std::ios::binary);
    if (!f) return tags;

    // Check JPEG SOI marker
    uint8_t soi[2];
    f.read(reinterpret_cast<char*>(soi), 2);
    if (soi[0] != 0xFF || soi[1] != 0xD8) return tags;  // Not JPEG

    // Search for APP1 marker (0xFF 0xE1)
    while (f.good()) {
        uint8_t marker[2];
        f.read(reinterpret_cast<char*>(marker), 2);
        if (marker[0] != 0xFF) break;

        uint8_t len_bytes[2];
        f.read(reinterpret_cast<char*>(len_bytes), 2);
        uint16_t seg_len = (len_bytes[0] << 8) | len_bytes[1];

        if (marker[1] == 0xE1) {
            // Found APP1 — check for "Exif\0\0" header
            char exif_header[6];
            f.read(exif_header, 6);
            if (std::memcmp(exif_header, "Exif\0\0", 6) == 0) {
                tags["Format"] = "JPEG with EXIF";
                // A full parser would decode the TIFF IFD here.
                // For now, we note that EXIF data is present.
                tags["EXIF"] = "Present (use libexiv2 for detailed parsing)";
            }
            break;
        } else {
            // Skip this segment
            f.seekg(seg_len - 2, std::ios::cur);
        }
    }

    // Always add file size
    std::error_code ec;
    auto fsize = std::filesystem::file_size(path, ec);
    if (!ec) {
        tags["FileSize"] = std::to_string(fsize) + " bytes";
    }

    return tags;
}

} // namespace fo::core
