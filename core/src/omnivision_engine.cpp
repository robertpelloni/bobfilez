/// @file omnivision_engine.cpp
/// @brief Implementation of the OmniVision NLE.

#include "fo/core/omnivision_engine_interface.hpp"
#include "fo/core/registry.hpp"
#include <iostream>
#include <sstream>

namespace fo::core {

class OmniVisionEngineImpl : public IOmniVisionEngine {
    std::map<std::string, TimelineSequence> sequences_;

public:
    OmniVisionEngineImpl() {
        std::cout << "[OmniVision] Loading OpenTimelineIO (OTIO) and OpenColorIO (OCIO) modules...\n";
    }

    TimelineSequence create_sequence(const std::string& name) override {
        TimelineSequence seq;
        seq.name = name;
        seq.total_duration_sec = 0.0;
        return seq;
    }

    bool add_clip(TimelineSequence& seq, const std::filesystem::path& file, double at_time) override {
        MediaClip clip;
        clip.id = "clip_" + std::to_string(seq.clips.size());
        clip.file_path = file;
        clip.start_time_sec = 0.0;
        clip.end_time_sec = 10.0; // Mock 10s duration
        clip.timeline_pos = at_time;
        clip.track_index = 1;
        
        seq.clips.push_back(clip);
        seq.total_duration_sec = std::max(seq.total_duration_sec, at_time + clip.end_time_sec);
        std::cout << "[OmniVision] Added clip " << file.filename() << " to timeline at " << at_time << "s\n";
        return true;
    }

    bool export_otio(const TimelineSequence& seq, const std::filesystem::path& dest_path) override {
        std::cout << "[OmniVision] Exporting OpenTimelineIO format to " << dest_path << "\n";
        return true; // Stubbed for now
    }

    bool render_sequence(const TimelineSequence& seq, const std::filesystem::path& output_file) override {
        std::cout << "[OmniVision] Rendering video using FFmpeg and OpenColorIO...\n";
        std::ostringstream ffmpeg_cmd;
        
        // Mock FFmpeg command with OCIO filtering
        ffmpeg_cmd << "ffmpeg";
        for (const auto& clip : seq.clips) {
            ffmpeg_cmd << " -i \"" << clip.file_path.string() << "\"";
        }
        
        ffmpeg_cmd << " -vf \"lut3d='" << seq.global_grade.lut_path << "'\"";
        ffmpeg_cmd << " -c:v libx265 -crf 23 -y \"" << output_file.string() << "\"";
        
        std::cout << "             Command: " << ffmpeg_cmd.str() << "\n";
        return true;
    }

    std::map<std::string, std::string> get_media_info(const std::filesystem::path& file) override {
        std::map<std::string, std::string> info;
        // Mock MediaInfo output
        info["Codec"] = "HEVC/H.265";
        info["Resolution"] = "3840x2160 (4K)";
        info["Framerate"] = "59.94 fps";
        info["ColorSpace"] = "Rec.2020 PQ";
        info["Audio"] = "AAC 320kbps 48kHz Stereo";
        return info;
    }
};

static auto reg = []() {
    Registry<IOmniVisionEngine>::instance().add("default", []() {
        return std::make_unique<OmniVisionEngineImpl>();
    });
    return true;
}();

} // namespace fo::core
