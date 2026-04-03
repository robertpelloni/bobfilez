#pragma once
/// @file omnivision_engine_interface.hpp
/// @brief Professional Non-Linear Editor (NLE) and Color Grading for bobfilez.
///
/// OmniVision utilizes OpenTimelineIO (OTIO) to arrange video/audio clips
/// into a sequence, OpenColorIO (OCIO) for cinematic color management, and 
/// mpv/FFmpeg for frame-accurate playback and rendering.

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

namespace fo::core {

struct MediaClip {
    std::string id;
    std::filesystem::path file_path;
    double start_time_sec;  // In-point of the source file
    double end_time_sec;    // Out-point of the source file
    double timeline_pos;    // Position on the master timeline
    int track_index;        // V1, V2, A1, etc.
};

struct ColorGrade {
    std::string lut_path;   // Path to .cube or .3dl LUT
    double exposure_stops = 0.0;
    double contrast = 1.0;
    double saturation = 1.0;
    std::string ocio_colorspace_in = "sRGB";
    std::string ocio_colorspace_out = "Rec709";
};

struct TimelineSequence {
    std::string name;
    std::vector<MediaClip> clips;
    ColorGrade global_grade;
    double total_duration_sec;
};

class IOmniVisionEngine {
public:
    virtual ~IOmniVisionEngine() = default;

    /// Create a new empty timeline sequence
    virtual TimelineSequence create_sequence(const std::string& name) = 0;

    /// Add a media file to the timeline
    virtual bool add_clip(TimelineSequence& seq, const std::filesystem::path& file, double at_time) = 0;

    /// Export the timeline to a standard OTIO format (OpenTimelineIO)
    virtual bool export_otio(const TimelineSequence& seq, const std::filesystem::path& dest_path) = 0;

    /// Render the final video using FFmpeg and OpenColorIO filters
    virtual bool render_sequence(const TimelineSequence& seq, const std::filesystem::path& output_file) = 0;

    /// Query media info (resolution, fps, codecs) using the MediaInfo submodule
    virtual std::map<std::string, std::string> get_media_info(const std::filesystem::path& file) = 0;
};

} // namespace fo::core
