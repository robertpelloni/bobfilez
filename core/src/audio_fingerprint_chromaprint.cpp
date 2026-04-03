/// @file audio_fingerprint_chromaprint.cpp
/// @brief Chromaprint-based audio fingerprinting implementation.
///
/// This provider uses FFmpeg to decode audio streams and Chromaprint
/// to generate AcoustID-compatible audio fingerprints.
///
/// Feature guard: FO_HAVE_CHROMAPRINT && FO_HAVE_FFMPEG
/// When these are not available, the provider is not registered.
///
/// Dependencies: chromaprint, libavformat, libavcodec, libavutil, libswresample

#include "fo/core/audio_fingerprint_interface.hpp"
#include "fo/core/registry.hpp"
#include <stdexcept>
#include <algorithm>
#include <cmath>

namespace fo::core {

// --- AudioFingerprint::similarity implementation ---
double AudioFingerprint::similarity(const AudioFingerprint& other) const {
    if (fingerprints.empty() || other.fingerprints.empty()) return 0.0;

    const auto& shorter = (fingerprints.size() < other.fingerprints.size()) ? fingerprints : other.fingerprints;
    const auto& longer = (fingerprints.size() >= other.fingerprints.size()) ? fingerprints : other.fingerprints;

    if (shorter.empty()) return 0.0;

    int best_score = 0;
    int max_possible_score = static_cast<int>(shorter.size() * 32);

    // Slide the shorter fingerprint across the longer one to find the best alignment
    // (Cross-correlation)
    int max_offset = static_cast<int>(longer.size() - shorter.size());
    for (int offset = 0; offset <= max_offset; ++offset) {
        int current_score = 0;
        for (size_t i = 0; i < shorter.size(); ++i) {
            uint32_t a = shorter[i];
            uint32_t b = longer[i + offset];
            uint32_t xor_val = a ^ b;
            
            // Count matching bits (32 - differing bits)
            int diff_bits = 0;
            uint32_t temp = xor_val;
            while (temp) {
                temp &= (temp - 1);
                ++diff_bits;
            }
            current_score += (32 - diff_bits);
        }
        if (current_score > best_score) {
            best_score = current_score;
        }
    }

    return static_cast<double>(best_score) / max_possible_score;
}

} // namespace fo::core

#if defined(FO_HAVE_CHROMAPRINT) && defined(FO_HAVE_FFMPEG)

#include <chromaprint.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

namespace fo::core {

class ChromaprintAudioFingerprinter : public IAudioFingerprinter {
public:
    std::string name() const override { return "chromaprint"; }

    AudioFingerprint fingerprint(const std::filesystem::path& path, double max_duration_sec) override {
        AudioFingerprint result;
        result.path = path;

        AVFormatContext* fmt_ctx = nullptr;
        if (avformat_open_input(&fmt_ctx, path.string().c_str(), nullptr, nullptr) < 0) {
            throw std::runtime_error("ChromaprintAudioFingerprinter: cannot open " + path.string());
        }

        struct FmtGuard {
            AVFormatContext* ctx;
            ~FmtGuard() { if (ctx) avformat_close_input(&ctx); }
        } fmt_guard{fmt_ctx};

        if (avformat_find_stream_info(fmt_ctx, nullptr) < 0) {
            throw std::runtime_error("ChromaprintAudioFingerprinter: cannot find stream info");
        }

        int audio_stream = -1;
        const AVCodec* codec = nullptr;
        audio_stream = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
        if (audio_stream < 0) {
            throw std::runtime_error("ChromaprintAudioFingerprinter: no audio stream found");
        }

        AVStream* stream = fmt_ctx->streams[audio_stream];
        AVCodecParameters* codecpar = stream->codecpar;

        result.sample_rate = codecpar->sample_rate;
        result.channels = codecpar->ch_layout.nb_channels;
        result.codec = avcodec_get_name(codecpar->codec_id);
        result.bitrate = codecpar->bit_rate;

        if (stream->duration != AV_NOPTS_VALUE) {
            result.duration_sec = stream->duration * av_q2d(stream->time_base);
        } else if (fmt_ctx->duration != AV_NOPTS_VALUE) {
            result.duration_sec = fmt_ctx->duration / static_cast<double>(AV_TIME_BASE);
        }

        AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
        if (!codec_ctx) {
            throw std::runtime_error("ChromaprintAudioFingerprinter: cannot allocate codec context");
        }
        struct CodecGuard {
            AVCodecContext* ctx;
            ~CodecGuard() { if (ctx) avcodec_free_context(&ctx); }
        } codec_guard{codec_ctx};

        avcodec_parameters_to_context(codec_ctx, codecpar);

        if (avcodec_open2(codec_ctx, codec, nullptr) < 0) {
            throw std::runtime_error("ChromaprintAudioFingerprinter: cannot open codec");
        }

        // Setup Chromaprint context
        ChromaprintContext* cp_ctx = chromaprint_new(CHROMAPRINT_ALGORITHM_DEFAULT);
        if (!cp_ctx) {
            throw std::runtime_error("ChromaprintAudioFingerprinter: cannot create chromaprint context");
        }
        struct CpGuard {
            ChromaprintContext* ctx;
            ~CpGuard() { if (ctx) chromaprint_free(ctx); }
        } cp_guard{cp_ctx};

        // We want to resample audio to 11025Hz Mono s16 for Chromaprint
        int target_sample_rate = 11025;
        int target_channels = 1;
        chromaprint_start(cp_ctx, target_sample_rate, target_channels);

        SwrContext* swr_ctx = nullptr;
        swr_alloc_set_opts2(&swr_ctx,
            &codec_ctx->ch_layout, AV_SAMPLE_FMT_S16, target_sample_rate,
            &codec_ctx->ch_layout, codec_ctx->sample_fmt, codec_ctx->sample_rate,
            0, nullptr);
        
        // Force mono
        av_opt_set_chlayout(swr_ctx, "out_chlayout", &codec_ctx->ch_layout, 0); // Need proper mono layout
        AVChannelLayout mono_layout = AV_CHANNEL_LAYOUT_MONO;
        swr_alloc_set_opts2(&swr_ctx,
            &mono_layout, AV_SAMPLE_FMT_S16, target_sample_rate,
            &codec_ctx->ch_layout, codec_ctx->sample_fmt, codec_ctx->sample_rate,
            0, nullptr);

        if (!swr_ctx || swr_init(swr_ctx) < 0) {
            if (swr_ctx) swr_free(&swr_ctx);
            throw std::runtime_error("ChromaprintAudioFingerprinter: cannot initialize resampler");
        }
        struct SwrGuard {
            SwrContext* ctx;
            ~SwrGuard() { if (ctx) swr_free(&ctx); }
        } swr_guard{swr_ctx};

        AVFrame* frame = av_frame_alloc();
        AVPacket* pkt = av_packet_alloc();
        struct FrameGuard {
            AVFrame* f; AVPacket* p;
            ~FrameGuard() { av_frame_free(&f); av_packet_free(&p); }
        } frame_guard{frame, pkt};

        int max_samples = swr_get_out_samples(swr_ctx, 8192);
        uint8_t* output_buffer = nullptr;
        av_samples_alloc(&output_buffer, nullptr, target_channels, max_samples, AV_SAMPLE_FMT_S16, 0);

        struct BufGuard {
            uint8_t* buf;
            ~BufGuard() { if (buf) av_freep(&buf); }
        } buf_guard{output_buffer};

        double processed_sec = 0.0;

        while (av_read_frame(fmt_ctx, pkt) >= 0) {
            if (pkt->stream_index != audio_stream) {
                av_packet_unref(pkt);
                continue;
            }

            avcodec_send_packet(codec_ctx, pkt);
            av_packet_unref(pkt);

            while (avcodec_receive_frame(codec_ctx, frame) == 0) {
                int out_samples = swr_get_out_samples(swr_ctx, frame->nb_samples);
                if (out_samples > max_samples) {
                    av_freep(&output_buffer);
                    max_samples = out_samples;
                    av_samples_alloc(&output_buffer, nullptr, target_channels, max_samples, AV_SAMPLE_FMT_S16, 0);
                    buf_guard.buf = output_buffer;
                }

                int converted_samples = swr_convert(swr_ctx, &output_buffer, max_samples,
                                                    (const uint8_t**)frame->data, frame->nb_samples);

                if (converted_samples > 0) {
                    chromaprint_feed(cp_ctx, (int16_t*)output_buffer, converted_samples);
                    processed_sec += (double)converted_samples / target_sample_rate;
                }

                if (max_duration_sec > 0 && processed_sec >= max_duration_sec) {
                    break;
                }
            }

            if (max_duration_sec > 0 && processed_sec >= max_duration_sec) {
                break;
            }
        }

        chromaprint_finish(cp_ctx);

        uint32_t* fprint = nullptr;
        int size = 0;
        if (chromaprint_get_raw_fingerprint(cp_ctx, &fprint, &size)) {
            result.fingerprints.assign(fprint, fprint + size);
            chromaprint_dealloc(fprint);
        }

        return result;
    }
};

static auto reg_chromaprint = []() {
    Registry<IAudioFingerprinter>::instance().add("chromaprint", []() {
        return std::make_unique<ChromaprintAudioFingerprinter>();
    });
    return true;
}();

} // namespace fo::core

#endif // FO_HAVE_CHROMAPRINT && FO_HAVE_FFMPEG
