#pragma once
/// @file image_viewer_interface.hpp
/// @brief XnViewMP-inspired image viewer/browser backend for bobfilez.
///
/// Modeled on XnViewMP's feature set (the gold standard for image viewers):
///   - Format support: 500+ read formats via ImageMagick/libvips/OpenCV/LibRaw
///   - Navigation: prev/next/first/last, folder browse, recursive browse
///   - Zoom: fit/fill/1:1/custom%, smooth/sharp interpolation
///   - Slideshow: configurable interval, transitions, random order
///   - Color tools: histogram, levels, curves, brightness/contrast
///   - Metadata: full EXIF/IPTC/XMP display and editing
///   - Batch tools: batch convert, batch rename, batch resize
///   - Compare mode: side-by-side image comparison
///   - RAW support: LibRaw-based demosaicing with white balance controls
///   - Lossless JPEG rotation/flip (jpegtran-compatible)
///   - Thumbnail cache: fast thumbnail generation and LRU disk cache

#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>
#include <optional>
#include <functional>
#include <map>
#include <chrono>

namespace fo::core {

//───────────────────────────── Image Info ────────────────────────────────────

struct ImageInfo {
    std::filesystem::path path;
    int width = 0, height = 0;
    int channels = 0;            // 1=gray, 3=RGB, 4=RGBA
    int bit_depth = 8;           // 8, 10, 12, 16, 32
    std::string format;          // "JPEG", "PNG", "RAW/CR2", etc.
    std::string color_profile;   // "sRGB", "Adobe RGB", "P3", etc.
    std::string compression;     // "DEFLATE", "LZW", etc.
    int64_t file_size = 0;
    std::chrono::system_clock::time_point modified;
    bool has_exif = false;
    bool has_alpha = false;
    bool is_animated = false;
    int frame_count = 1;
    double dpi_x = 72.0, dpi_y = 72.0;
    std::map<std::string, std::string> exif_tags;  // Key → human-readable value
    std::map<std::string, std::string> iptc_tags;
    std::map<std::string, std::string> xmp_tags;
    std::string gps_latitude, gps_longitude, gps_altitude;
    std::string camera_make, camera_model, lens_model;
    std::string date_taken;       // ISO 8601
    std::string focal_length, aperture, shutter_speed, iso;
};

//───────────────────────────── Thumbnail Cache ───────────────────────────────

/// LRU thumbnail cache — stores thumbnails in a configurable disk cache dir.
/// The cache key is a hash of the file path + mtime + size.
/// Thumbnails are stored as compressed JPEG blobs.
class ThumbnailCache {
public:
    explicit ThumbnailCache(const std::filesystem::path& cache_dir, int64_t max_size_bytes = 512LL * 1024 * 1024);

    /// Get a thumbnail (256x256 JPEG bytes). Generates if not cached.
    std::vector<uint8_t> get(const std::filesystem::path& image_path, int thumb_size = 256);

    /// Async generation: returns immediately, calls callback when ready
    void get_async(const std::filesystem::path& image_path, int thumb_size,
                   std::function<void(std::vector<uint8_t>)> cb);

    /// Purge entries for files that no longer exist
    int purge_orphans();
    /// Purge oldest entries until cache is within max_size_bytes
    int purge_old();
    int64_t cache_size_bytes() const;
    void clear();

private:
    std::filesystem::path cache_dir_;
    int64_t max_size_bytes_;
    std::string make_key(const std::filesystem::path& path) const;
};

//───────────────────────────── Histogram ─────────────────────────────────────

struct ChannelHistogram {
    std::vector<uint32_t> bins; // 256 bins (or 1024 for 16-bit)
    uint32_t max_count = 0;
    double mean = 0.0, std_dev = 0.0;
    uint8_t black_point = 0, white_point = 255;
    uint8_t median = 0;
};

struct ImageHistogram {
    ChannelHistogram red, green, blue, alpha, luminance;
    static ImageHistogram compute(const std::vector<uint8_t>& rgba_data, int width, int height);
};

//───────────────────────────── Color Adjustments ─────────────────────────────

struct ColorAdjustments {
    // Levels
    double input_black = 0.0, input_white = 255.0, gamma = 1.0;
    double output_black = 0.0, output_white = 255.0;
    // Basic
    double brightness = 0.0;     // -100 to +100
    double contrast = 0.0;       // -100 to +100
    double saturation = 0.0;     // -100 to +100
    double hue_shift = 0.0;      // -180 to +180 degrees
    double vibrance = 0.0;       // Smart saturation
    double sharpness = 0.0;      // 0-100
    double noise_reduction = 0.0;// 0-100
    // White balance
    double temperature = 0.0;    // Kelvin offset
    double tint = 0.0;           // Green-Magenta
    // Exposure (for RAW)
    double exposure = 0.0;       // EV stops (-5 to +5)
    double highlights = 0.0;
    double shadows = 0.0;
    double blacks = 0.0;
    double whites = 0.0;
    double clarity = 0.0;
    double dehaze = 0.0;
};

//───────────────────────────── Slideshow Options ─────────────────────────────

struct SlideshowOptions {
    double interval_sec = 5.0;
    bool loop = true;
    bool random_order = false;
    bool include_subfolders = false;
    enum class Transition { None, Fade, Slide, Zoom, Dissolve } transition = Transition::Fade;
    double transition_duration_sec = 0.5;
    bool show_filename = true;
    bool show_metadata = false;
    bool pause_on_hover = true;
    enum class FitMode { FitPage, FillPage, OneToOne, Custom } fit_mode = FitMode::FitPage;
    bool stretch_small = false;   // Upscale images smaller than screen
};

//───────────────────────────── Image Viewer Engine ───────────────────────────

/// Supported image extensions (read-only and read-write)
struct ImageFormatSupport {
    std::string ext;
    bool can_read = true;
    bool can_write = false;
    bool raw = false;    // Camera RAW
    std::string description;
};

class ImageViewerEngine {
public:
    /// Get info for a folder of images (for browser/filmstrip view)
    std::vector<ImageInfo> browse_folder(
        const std::filesystem::path& dir,
        bool recursive = false,
        const std::vector<std::string>& extensions = {});

    /// Load detailed metadata for a single image
    ImageInfo get_info(const std::filesystem::path& path);

    /// Decode image to RGBA bytes for display
    /// Scale to max_dim if > 0 (for thumbnails or view-fitting)
    struct DecodedImage {
        std::vector<uint8_t> rgba;
        int width = 0, height = 0;
        int original_width = 0, original_height = 0;
        ImageInfo info;
    };
    DecodedImage decode(const std::filesystem::path& path,
                         int max_dim = 0,
                         const ColorAdjustments& adjustments = {});

    /// Compute histogram from decoded image
    ImageHistogram compute_histogram(const DecodedImage& img);

    /// Rotate image (lossless for JPEG, lossy otherwise)
    bool rotate(const std::filesystem::path& path, int degrees_cw, bool lossless = true);

    /// Flip image (lossless for JPEG)
    bool flip(const std::filesystem::path& path, bool horizontal, bool lossless = true);

    /// Apply color adjustments and save to new path (or overwrite)
    bool apply_adjustments(const std::filesystem::path& input,
                            const std::filesystem::path& output,
                            const ColorAdjustments& adj);

    /// Write/update EXIF/IPTC/XMP tags (uses exiv2)
    bool write_metadata(const std::filesystem::path& path,
                         const std::map<std::string, std::string>& tags);
    bool strip_metadata(const std::filesystem::path& path);

    /// All supported formats
    static std::vector<ImageFormatSupport> supported_formats();

    /// Get the thumbnail cache
    ThumbnailCache& thumbnail_cache();

private:
    std::optional<ThumbnailCache> cache_;
};

} // namespace fo::core
