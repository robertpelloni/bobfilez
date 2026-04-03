#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>

namespace fo::core {

/// Represents a single frame's perceptual hash for video analysis.
/// Each frame hash is a 64-bit value computed via dHash (difference hash)
/// of a downsampled grayscale frame.
struct FrameHash {
    int frame_number;        // Frame index in the video
    double timestamp_sec;    // Timestamp in seconds
    uint64_t hash;           // 64-bit perceptual hash of the frame
};

/// Result of video perceptual hashing for a single video file.
/// Contains metadata about the video and a sequence of frame hashes
/// that can be compared against other videos for similarity.
struct VideoHashResult {
    std::filesystem::path path;       // Path to the video file
    int total_frames = 0;             // Total frame count reported by the video
    double duration_sec = 0.0;        // Duration in seconds
    int width = 0;                    // Video width in pixels
    int height = 0;                   // Video height in pixels
    double fps = 0.0;                 // Frames per second
    std::string codec;                // Codec name (e.g., "h264", "hevc")
    std::vector<FrameHash> frame_hashes; // Sampled frame hashes

    /// Compute the average Hamming distance between this video's frame hashes
    /// and another video's frame hashes. Lower values indicate more similarity.
    /// Returns a normalized similarity score in [0.0, 1.0] where 1.0 = identical.
    ///
    /// Algorithm:
    ///   1. Align frame sequences by normalized timestamp (0.0 to 1.0).
    ///   2. For each sampled frame in `this`, find the nearest-timestamp frame in `other`.
    ///   3. Compute Hamming distance between matched frame pairs.
    ///   4. Average all distances and normalize to [0, 64] range.
    ///   5. Return (64 - avg_distance) / 64.0 as similarity.
    ///
    /// This approach is resilient to:
    ///   - Different resolutions (frames are downsampled before hashing)
    ///   - Different codecs/containers (we hash raw pixel data)
    ///   - Minor edits (watermarks, color grading) if they don't drastically change frames
    ///   - Different framerates (timestamp-based alignment)
    double similarity(const VideoHashResult& other) const;
};

/// Interface for video perceptual hashing providers.
/// Implementations extract frames from video files and compute
/// perceptual hashes for duplicate/similar video detection.
///
/// Design rationale:
///   - Uses FFmpeg (libav*) for frame extraction since it supports virtually
///     all video formats and codecs in existence.
///   - Samples frames at regular intervals rather than hashing every frame
///     to keep processing time reasonable for large video libraries.
///   - Uses dHash (difference hash) per frame, which is fast and resilient
///     to minor visual changes like re-encoding artifacts.
///
/// Usage:
///   auto provider = Registry<IVideoHasher>::instance().create("ffmpeg");
///   auto result = provider->hash_video("movie.mp4", 32); // 32 sample frames
///   double sim = result.similarity(other_result);
///   if (sim > 0.85) { /* likely duplicate */ }
class IVideoHasher {
public:
    virtual ~IVideoHasher() = default;

    /// Hash a video file by extracting and hashing sampled frames.
    ///
    /// @param path         Path to the video file
    /// @param sample_count Number of frames to sample evenly across the video duration.
    ///                     Higher values increase accuracy but also processing time.
    ///                     Recommended: 16-64 for quick scans, 128+ for high accuracy.
    /// @return VideoHashResult containing metadata and frame hashes
    /// @throws std::runtime_error if the video cannot be opened or decoded
    virtual VideoHashResult hash_video(const std::filesystem::path& path, int sample_count = 32) = 0;

    /// Get the name of this video hasher implementation.
    virtual std::string name() const = 0;
};

/// Compute the Hamming distance between two 64-bit hashes.
/// Returns the number of differing bits (0 = identical, 64 = maximally different).
/// This is the fundamental operation for comparing perceptual hashes.
inline int hamming_distance_64(uint64_t a, uint64_t b) {
    uint64_t x = a ^ b;
    int count = 0;
    // Brian Kernighan's bit counting algorithm - O(number of set bits)
    while (x) {
        x &= (x - 1);
        ++count;
    }
    return count;
}

} // namespace fo::core
