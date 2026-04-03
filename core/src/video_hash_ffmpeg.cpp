/// @file video_hash_ffmpeg.cpp
/// @brief FFmpeg-based video perceptual hashing implementation.
///
/// This provider uses FFmpeg's libav* libraries to decode video files,
/// extract sampled frames, downscale them to 9x8 grayscale thumbnails,
/// and compute dHash (difference hash) for each frame.
///
/// The dHash algorithm works by:
///   1. Resize frame to 9x8 pixels (9 wide so we get 8 horizontal differences)
///   2. Convert to grayscale
///   3. For each row, compare adjacent pixels left-to-right
///   4. If left pixel > right pixel, set bit = 1, else bit = 0
///   5. This produces 8*8 = 64 bits = one uint64_t hash
///
/// Feature guard: FO_HAVE_FFMPEG
/// When FFmpeg is not available, the provider is not registered and
/// attempting to use video hashing will fail gracefully.
///
/// Dependencies: libavformat, libavcodec, libavutil, libswscale

#include "fo/core/video_hash_interface.hpp"
#include "fo/core/registry.hpp"
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <numeric>

namespace fo::core {

// --- VideoHashResult::similarity implementation ---
// This lives here regardless of FFmpeg availability since it's pure computation.
double VideoHashResult::similarity(const VideoHashResult& other) const {
    if (frame_hashes.empty() || other.frame_hashes.empty()) return 0.0;

    // For each of our frames, find the closest-timestamp frame in `other`
    // and compute Hamming distance between matched pairs.
    double total_distance = 0.0;
    int matched = 0;

    for (const auto& fh : frame_hashes) {
        // Normalize our timestamp to [0, 1] range based on video duration
        double norm_ts = (duration_sec > 0) ? (fh.timestamp_sec / duration_sec) : 0.0;

        // Find the nearest frame in the other video by normalized timestamp
        const FrameHash* best = nullptr;
        double best_dt = 1e9;
        for (const auto& ofh : other.frame_hashes) {
            double other_norm = (other.duration_sec > 0) ? (ofh.timestamp_sec / other.duration_sec) : 0.0;
            double dt = std::abs(norm_ts - other_norm);
            if (dt < best_dt) {
                best_dt = dt;
                best = &ofh;
            }
        }

        if (best && best_dt < 0.1) { // Only match if timestamps are within 10% of each other
            total_distance += hamming_distance_64(fh.hash, best->hash);
            ++matched;
        }
    }

    if (matched == 0) return 0.0;

    double avg_distance = total_distance / matched;
    // Normalize: 0 distance = 1.0 similarity, 64 distance = 0.0 similarity
    return std::max(0.0, (64.0 - avg_distance) / 64.0);
}

} // namespace fo::core

#ifdef FO_HAVE_FFMPEG

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace fo::core {

/// FFmpeg-based video perceptual hasher.
/// Decodes video frames at evenly-spaced intervals and computes dHash for each.
///
/// Why dHash over pHash for video?
///   - dHash is significantly faster (no DCT computation)
///   - For video comparison, we have many frames, so statistical averaging
///     compensates for dHash's slightly lower per-frame discriminative power
///   - dHash is more resilient to brightness/contrast changes common in re-encodes
class FFmpegVideoHasher : public IVideoHasher {
public:
    std::string name() const override { return "ffmpeg"; }

    VideoHashResult hash_video(const std::filesystem::path& path, int sample_count) override {
        VideoHashResult result;
        result.path = path;

        // Open the input file
        AVFormatContext* fmt_ctx = nullptr;
        if (avformat_open_input(&fmt_ctx, path.string().c_str(), nullptr, nullptr) < 0) {
            throw std::runtime_error("FFmpegVideoHasher: cannot open " + path.string());
        }

        // RAII cleanup for format context
        struct FmtGuard {
            AVFormatContext* ctx;
            ~FmtGuard() { if (ctx) avformat_close_input(&ctx); }
        } fmt_guard{fmt_ctx};

        // Find stream info
        if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
            throw std::runtime_error("FFmpegVideoHasher: cannot find stream info");
        }

        // Find the best video stream
        int video_stream = -1;
        const AVCodec* codec = nullptr;
        video_stream = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &codec, 0);
        if (video_stream < 0) {
            throw std::runtime_error("FFmpegVideoHasher: no video stream found");
        }

        // Get stream parameters
        AVStream* stream = fmt_ctx->streams[video_stream];
        AVCodecParameters* codecpar = stream->codecpar;

        result.width = codecpar->width;
        result.height = codecpar->height;
        result.codec = avcodec_get_name(codecpar->codec_id);

        // Compute duration and FPS
        if (stream->duration != AV_NOPTS_VALUE) {
            result.duration_sec = stream->duration * av_q2d(stream->time_base);
        } else if (fmt_ctx->duration != AV_NOPTS_VALUE) {
            result.duration_sec = fmt_ctx->duration / static_cast<double>(AV_TIME_BASE);
        }

        if (stream->avg_frame_rate.den > 0) {
            result.fps = av_q2d(stream->avg_frame_rate);
        }

        if (result.fps > 0 && result.duration_sec > 0) {
            result.total_frames = static_cast<int>(result.fps * result.duration_sec);
        }

        // Open codec context
        AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
        if (!codec_ctx) {
            throw std::runtime_error("FFmpegVideoHasher: cannot allocate codec context");
        }
        struct CodecGuard {
            AVCodecContext* ctx;
            ~CodecGuard() { if (ctx) avcodec_free_context(&ctx); }
        } codec_guard{codec_ctx};

        avcodec_parameters_to_context(codec_ctx, codecpar);

        if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
            throw std::runtime_error("FFmpegVideoHasher: cannot open codec");
        }

        // Set up scaler to convert frames to 9x8 grayscale for dHash.
        // We need 9 columns to compute 8 horizontal differences per row.
        constexpr int DHASH_W = 9;
        constexpr int DHASH_H = 8;

        SwsContext* sws = sws_getContext(
            codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
            DHASH_W, DHASH_H, AV_PIX_FMT_GRAY8,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );
        if (!sws) {
            throw std::runtime_error("FFmpegVideoHasher: cannot create scaler");
        }
        struct SwsGuard {
            SwsContext* ctx;
            ~SwsGuard() { if (ctx) sws_freeContext(ctx); }
        } sws_guard{sws};

        // Allocate frames
        AVFrame* frame = av_frame_alloc();
        AVFrame* gray_frame = av_frame_alloc();
        AVPacket* pkt = av_packet_alloc();

        struct FrameGuard {
            AVFrame* f1; AVFrame* f2; AVPacket* p;
            ~FrameGuard() { av_frame_free(&f1); av_frame_free(&f2); av_packet_free(&p); }
        } frame_guard{frame, gray_frame, pkt};

        // Allocate buffer for grayscale output frame
        uint8_t gray_buf[DHASH_W * DHASH_H];
        gray_frame->data[0] = gray_buf;
        gray_frame->linesize[0] = DHASH_W;

        // Calculate which timestamps to sample.
        // We distribute sample_count frames evenly across the video duration.
        // Skip the first and last 1% to avoid black frames from intros/credits.
        std::vector<double> target_times;
        if (result.duration_sec > 0 && sample_count > 0) {
            double start = result.duration_sec * 0.01; // Skip first 1%
            double end = result.duration_sec * 0.99;   // Skip last 1%
            double step = (end - start) / std::max(1, sample_count - 1);
            for (int i = 0; i < sample_count; ++i) {
                target_times.push_back(start + i * step);
            }
        }

        // Seek to each target time and decode one frame
        for (size_t ti = 0; ti < target_times.size(); ++ti) {
            double target_sec = target_times[ti];
            int64_t target_ts = static_cast<int64_t>(target_sec / av_q2d(stream->time_base));

            // Seek to the nearest keyframe before our target
            av_seek_frame(fmt_ctx, video_stream, target_ts, AVSEEK_FLAG_BACKWARD);
            avcodec_flush_buffers(codec_ctx);

            // Decode frames until we reach or pass our target timestamp
            bool got_frame = false;
            while (av_read_frame(fmt_ctx, pkt) >= 0) {
                if (pkt->stream_index != video_stream) {
                    av_packet_unref(pkt);
                    continue;
                }

                avcodec_send_packet(codec_ctx, pkt);
                av_packet_unref(pkt);

                while (avcodec_receive_frame(codec_ctx, frame) == 0) {
                    double frame_ts = frame->best_effort_timestamp * av_q2d(stream->time_base);
                    if (frame_ts >= target_sec - 0.1) {
                        // Scale frame down to 9x8 grayscale
                        sws_scale(sws,
                            frame->data, frame->linesize, 0, frame->height,
                            gray_frame->data, gray_frame->linesize
                        );

                        // Compute dHash from the 9x8 grayscale thumbnail.
                        // For each of 8 rows, compare 8 pairs of adjacent pixels.
                        // If pixel[col] > pixel[col+1], set bit.
                        uint64_t hash = 0;
                        for (int row = 0; row < DHASH_H; ++row) {
                            for (int col = 0; col < DHASH_W - 1; ++col) {
                                int idx = row * DHASH_W + col;
                                if (gray_buf[idx] > gray_buf[idx + 1]) {
                                    hash |= (1ULL << (row * 8 + col));
                                }
                            }
                        }

                        FrameHash fh;
                        fh.frame_number = static_cast<int>(ti);
                        fh.timestamp_sec = frame_ts;
                        fh.hash = hash;
                        result.frame_hashes.push_back(fh);

                        got_frame = true;
                        break;
                    }
                }
                if (got_frame) break;
            }
        }

        return result;
    }
};

// Register the FFmpeg video hasher with the provider registry.
// This allows CLI/GUI code to instantiate it via:
//   Registry<IVideoHasher>::instance().create("ffmpeg")
static auto reg_ffmpeg_vhash = []() {
    Registry<IVideoHasher>::instance().add("ffmpeg", []() {
        return std::make_unique<FFmpegVideoHasher>();
    });
    return true;
}();

} // namespace fo::core

#endif // FO_HAVE_FFMPEG
