#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>

namespace fo::core {

/// Result of audio fingerprinting for a single audio/video file.
/// Contains metadata and a sequence of audio "chroma" fingerprints
/// that can be compared for duplicate/similar audio detection.
///
/// Audio fingerprinting works differently from video hashing:
///   - Audio is converted to a chromagram (12-bin chroma feature per time window)
///   - Each chroma vector represents the tonal content at a point in time
///   - Comparison uses cross-correlation to handle time offsets
///   - This approach is resilient to re-encoding, volume changes, and format conversion
struct AudioFingerprint {
    std::filesystem::path path;          // Path to the audio/video file
    double duration_sec = 0.0;           // Duration in seconds
    int sample_rate = 0;                 // Sample rate (e.g., 44100)
    int channels = 0;                    // Number of audio channels
    std::string codec;                   // Audio codec (e.g., "aac", "mp3", "flac")
    int bitrate = 0;                     // Bitrate in bps
    std::vector<uint32_t> fingerprints;  // Sequence of sub-fingerprint codes

    /// Compute similarity between two audio fingerprints.
    /// Returns a normalized score in [0.0, 1.0] where 1.0 = identical audio.
    ///
    /// Algorithm:
    ///   1. Compute bit-error rate between aligned fingerprint sequences.
    ///   2. Try multiple time offsets to handle different start times.
    ///   3. Return the best (lowest error) alignment as similarity.
    ///
    /// Resilient to:
    ///   - Different bitrates and codecs (we analyze decoded PCM)
    ///   - Volume normalization differences
    ///   - Minor edits (intro/outro differences handled by offset alignment)
    double similarity(const AudioFingerprint& other) const;
};

/// Interface for audio fingerprinting providers.
/// Implementations decode audio from files and generate fingerprints
/// for duplicate/similar audio detection.
///
/// Design rationale:
///   - Uses Chromaprint/AcoustID algorithm for fingerprint generation
///   - Works on both pure audio files (MP3, FLAC, WAV) and video files
///     (extracts audio stream)
///   - Fingerprints are compact (one uint32 per ~0.1 sec of audio)
///   - Designed for local-first operation, no cloud API needed
///
/// Usage:
///   auto provider = Registry<IAudioFingerprinter>::instance().create("chromaprint");
///   auto fp1 = provider->fingerprint("song_128kbps.mp3");
///   auto fp2 = provider->fingerprint("song_320kbps.mp3");
///   double sim = fp1.similarity(fp2);
///   if (sim > 0.90) { /* same audio, different encoding */ }
class IAudioFingerprinter {
public:
    virtual ~IAudioFingerprinter() = default;

    /// Generate an audio fingerprint for the given file.
    ///
    /// @param path      Path to an audio or video file
    /// @param max_duration_sec Maximum audio duration to analyze (0 = full file).
    ///                         Limiting to 120s is often sufficient for identification
    ///                         and dramatically reduces processing time for long files.
    /// @return AudioFingerprint containing metadata and fingerprint codes
    /// @throws std::runtime_error if the file cannot be opened or decoded
    virtual AudioFingerprint fingerprint(const std::filesystem::path& path, double max_duration_sec = 120.0) = 0;

    /// Get the name of this audio fingerprinter implementation.
    virtual std::string name() const = 0;
};

} // namespace fo::core
