#pragma once
/// @file omniaudio_engine_interface.hpp
/// @brief Professional Digital Audio Workstation (DAW) & AI Audio Engine.
///
/// OmniAudio provides multitrack mixing, DSP effects, and advanced AI
/// audio processing (Stem Separation, Acoustic Fingerprinting).

#include <string>
#include <vector>
#include <memory>
#include <filesystem>

namespace fo::core {

struct AudioTrack {
    std::string id;
    std::string name;
    double volume_db = 0.0;
    double pan = 0.0;
    bool is_muted = false;
    bool is_solo = false;
    std::vector<std::string> loaded_plugins; // VSTs or built-in DSP
};

struct AudioProject {
    std::string name;
    int bpm = 120;
    int sample_rate = 48000;
    std::vector<AudioTrack> tracks;
};

struct SongIdentity {
    std::string title;
    std::string artist;
    std::string album;
    std::string genre;
    double confidence;
};

struct StemPaths {
    std::filesystem::path vocals;
    std::filesystem::path drums;
    std::filesystem::path bass;
    std::filesystem::path other;
};

class IOmniAudioEngine {
public:
    virtual ~IOmniAudioEngine() = default;

    /// Create a new multitrack audio project
    virtual AudioProject create_project(const std::string& name, int bpm) = 0;

    /// Add a new track to the project
    virtual bool add_track(AudioProject& project, const std::string& track_name) = 0;

    /// Identify a song using acoustic fingerprinting (ShazamIO/Chromaprint)
    virtual SongIdentity identify_song(const std::filesystem::path& file) = 0;

    /// Run AI Stem Separation (Audiocraft / Demucs) to split a track into instruments
    virtual StemPaths separate_stems(const std::filesystem::path& input_file, const std::filesystem::path& out_dir) = 0;

    /// Render the final mixed audio project
    virtual bool render_mixdown(const AudioProject& project, const std::filesystem::path& output_file) = 0;
};

} // namespace fo::core
