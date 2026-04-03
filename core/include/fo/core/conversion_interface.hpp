#pragma once
/// @file conversion_interface.hpp
/// @brief Universal file format conversion interfaces for bobfilez.
///
/// This module provides the backbone for "convert anything to anything"
/// functionality. The design uses a registry of IConverter implementations
/// — one per tool backend (FFmpeg, ImageMagick, Pandoc, Ghostscript, etc.)
/// Each converter declares the MIME/extension pairs it can handle.
/// The ConversionEngine picks the right converter automatically.
///
/// Supported conversion categories:
///   Video:   FFmpeg  (mp4/mkv/avi/mov/webm/flv/gif ↔ any)
///   Audio:   FFmpeg  (mp3/flac/wav/aac/ogg/opus/m4a ↔ any)
///   Image:   ImageMagick / libvips (png/jpg/webp/avif/heic/tiff/bmp/ico ↔ any)
///   RAW:     LibRaw → ImageMagick   (cr2/nef/arw/dng → jpg/png/tiff)
///   PDF:     Ghostscript / Poppler  (pdf ↔ ps/eps/png/svg)
///   Doc:     Pandoc                 (md/rst/docx/odt/html/tex/epub ↔ any)
///   Ebook:   Calibre CLI            (epub/mobi/azw3/fb2 ↔ any)
///   Vector:  Inkscape CLI           (svg ↔ pdf/png/emf)
///   Archive: libzip / libarchive    (zip/tar/7z/rar extraction/creation)

#include <string>
#include <vector>
#include <filesystem>
#include <functional>
#include <optional>
#include <map>

namespace fo::core {

/// A single conversion option the user can set (like video bitrate, image quality, etc.)
struct ConvertOption {
    std::string key;          // e.g. "quality", "bitrate", "fps", "page"
    std::string description;  // Human-readable description shown in UI tooltip
    std::string default_val;  // Default value as string
    std::string type;         // "int", "float", "string", "bool", "choice"
    std::vector<std::string> choices; // For type=="choice"
    std::string min_val;      // For numeric types
    std::string max_val;
};

/// Result of a single file conversion attempt.
struct ConversionResult {
    std::filesystem::path input;
    std::filesystem::path output;
    bool success = false;
    std::string error;        // Non-empty on failure
    double duration_sec = 0.0;// Wall-clock time taken
    int64_t input_size = 0;
    int64_t output_size = 0;
};

/// Progress callback for batch conversions.
/// Called with (current_index, total, current_file, result_so_far).
using ConversionProgressCb = std::function<void(int, int, const std::filesystem::path&, const ConversionResult*)>;

/// Abstract interface for a single converter backend.
class IConverter {
public:
    virtual ~IConverter() = default;

    /// Unique name for registry (e.g. "ffmpeg", "imagemagick", "pandoc")
    virtual std::string name() const = 0;

    /// Human-readable description
    virtual std::string description() const = 0;

    /// All input extensions this converter can handle (lowercase, no dot)
    virtual std::vector<std::string> supported_input_extensions() const = 0;

    /// All output extensions this converter can produce from a given input
    virtual std::vector<std::string> supported_output_extensions(const std::string& input_ext) const = 0;

    /// Available options for a given input→output conversion pair
    virtual std::vector<ConvertOption> available_options(const std::string& input_ext,
                                                          const std::string& output_ext) const = 0;

    /// Convert a single file. options_map is key→value from available_options().
    virtual ConversionResult convert(
        const std::filesystem::path& input,
        const std::filesystem::path& output,
        const std::map<std::string, std::string>& options = {}) = 0;
};

/// The ConversionEngine orchestrates multiple IConverter backends.
/// Responsible for:
///   - Picking the right backend for input→output extension pair
///   - Running batch conversions with progress reporting
///   - Generating output paths (same-dir, output-dir, rename patterns)
class ConversionEngine {
public:
    /// Register a converter backend
    void register_converter(std::shared_ptr<IConverter> conv);

    /// Find converters that can handle this input→output pair.
    /// Returns ordered list (best first).
    std::vector<std::shared_ptr<IConverter>> find_converters(
        const std::string& input_ext,
        const std::string& output_ext) const;

    /// Get all output extensions reachable from a given input extension
    std::vector<std::string> possible_outputs(const std::string& input_ext) const;

    /// Get all input extensions any registered converter can handle
    std::vector<std::string> all_input_extensions() const;

    /// Convert a single file using the best available converter
    ConversionResult convert(
        const std::filesystem::path& input,
        const std::string& output_ext,
        const std::filesystem::path& output_dir = {},
        const std::map<std::string, std::string>& options = {},
        const std::string& preferred_converter = "");

    /// Batch convert a list of files.
    /// output_dir: where to put results (empty = same dir as input).
    /// output_ext: target extension (e.g. "mp4").
    /// Returns results for each file.
    std::vector<ConversionResult> batch_convert(
        const std::vector<std::filesystem::path>& inputs,
        const std::string& output_ext,
        const std::filesystem::path& output_dir = {},
        const std::map<std::string, std::string>& options = {},
        int num_threads = 1,
        ConversionProgressCb progress_cb = nullptr,
        const std::string& preferred_converter = "");

private:
    std::vector<std::shared_ptr<IConverter>> converters_;
};

/// Global ConversionEngine singleton
ConversionEngine& global_conversion_engine();

} // namespace fo::core
