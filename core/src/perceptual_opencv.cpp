#include "fo/core/perceptual_hash_interface.hpp"
#include "fo/core/registry.hpp"

#ifdef FO_HAVE_OPENCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#endif

#include <iostream>
#include <cmath>

namespace fo::core {

#ifdef FO_HAVE_OPENCV

// dHash implementation - difference hash based on gradient direction
class DHashPerceptualHasher : public IPerceptualHasher {
public:
    std::string name() const override { return "dhash"; }

    std::optional<PerceptualHash> compute(const std::filesystem::path& image_path) override {
        try {
            cv::Mat img = cv::imread(image_path.string(), cv::IMREAD_GRAYSCALE);
            if (img.empty()) {
                return std::nullopt;
            }

            // Resize to 9x8 (9 columns, 8 rows)
            cv::Mat resized;
            cv::resize(img, resized, cv::Size(9, 8), 0, 0, cv::INTER_AREA);

            // Compute hash: compare adjacent pixels
            uint64_t hash_val = 0;
            for (int r = 0; r < 8; ++r) {
                const uint8_t* row_ptr = resized.ptr<uint8_t>(r);
                for (int c = 0; c < 8; ++c) {
                    if (row_ptr[c] > row_ptr[c + 1]) {
                        hash_val |= (1ULL << ((r * 8) + c));
                    }
                }
            }

            return PerceptualHash{hash_val, "dhash"};

        } catch (const std::exception& e) {
            std::cerr << "dHash error: " << e.what() << std::endl;
            return std::nullopt;
        }
    }
};

// pHash implementation - DCT-based perceptual hash
class PHashPerceptualHasher : public IPerceptualHasher {
public:
    std::string name() const override { return "phash"; }

    std::optional<PerceptualHash> compute(const std::filesystem::path& image_path) override {
        try {
            cv::Mat img = cv::imread(image_path.string(), cv::IMREAD_GRAYSCALE);
            if (img.empty()) {
                return std::nullopt;
            }

            // 1. Resize to 32x32
            cv::Mat resized;
            cv::resize(img, resized, cv::Size(32, 32), 0, 0, cv::INTER_AREA);

            // 2. Convert to float for DCT
            cv::Mat float_img;
            resized.convertTo(float_img, CV_32F);

            // 3. Apply DCT
            cv::Mat dct_result;
            cv::dct(float_img, dct_result);

            // 4. Extract top-left 8x8 (low frequencies)
            cv::Mat dct_low = dct_result(cv::Rect(0, 0, 8, 8));

            // 5. Compute mean (excluding DC component at [0,0])
            double sum = 0;
            for (int r = 0; r < 8; ++r) {
                const float* row_ptr = dct_low.ptr<float>(r);
                for (int c = 0; c < 8; ++c) {
                    if (r == 0 && c == 0) continue; // Skip DC
                    sum += row_ptr[c];
                }
            }
            double mean = sum / 63.0; // 64 - 1 (DC)

            // 6. Generate hash: 1 if above mean, 0 otherwise
            uint64_t hash_val = 0;
            for (int r = 0; r < 8; ++r) {
                const float* row_ptr = dct_low.ptr<float>(r);
                for (int c = 0; c < 8; ++c) {
                    if (row_ptr[c] > mean) {
                        hash_val |= (1ULL << ((r * 8) + c));
                    }
                }
            }

            return PerceptualHash{hash_val, "phash"};

        } catch (const std::exception& e) {
            std::cerr << "pHash error: " << e.what() << std::endl;
            return std::nullopt;
        }
    }
};

// aHash implementation - average hash (simple but fast)
class AHashPerceptualHasher : public IPerceptualHasher {
public:
    std::string name() const override { return "ahash"; }

    std::optional<PerceptualHash> compute(const std::filesystem::path& image_path) override {
        try {
            cv::Mat img = cv::imread(image_path.string(), cv::IMREAD_GRAYSCALE);
            if (img.empty()) {
                return std::nullopt;
            }

            // 1. Resize to 8x8
            cv::Mat resized;
            cv::resize(img, resized, cv::Size(8, 8), 0, 0, cv::INTER_AREA);

            // 2. Compute mean
            cv::Scalar mean_scalar = cv::mean(resized);
            double mean = mean_scalar[0];

            // 3. Generate hash: 1 if above mean, 0 otherwise
            uint64_t hash_val = 0;
            for (int r = 0; r < 8; ++r) {
                const uint8_t* row_ptr = resized.ptr<uint8_t>(r);
                for (int c = 0; c < 8; ++c) {
                    if (row_ptr[c] > mean) {
                        hash_val |= (1ULL << ((r * 8) + c));
                    }
                }
            }

            return PerceptualHash{hash_val, "ahash"};

        } catch (const std::exception& e) {
            std::cerr << "aHash error: " << e.what() << std::endl;
            return std::nullopt;
        }
    }
};

namespace {
    // Register all perceptual hash providers
    struct PerceptualHashRegistrar {
        PerceptualHashRegistrar() {
            Registry<IPerceptualHasher>::instance().add("dhash", []() {
                return std::make_unique<DHashPerceptualHasher>();
            });
            Registry<IPerceptualHasher>::instance().add("phash", []() {
                return std::make_unique<PHashPerceptualHasher>();
            });
            Registry<IPerceptualHasher>::instance().add("ahash", []() {
                return std::make_unique<AHashPerceptualHasher>();
            });
            // Keep "opencv" as alias for dhash for backward compatibility
            Registry<IPerceptualHasher>::instance().add("opencv", []() {
                return std::make_unique<DHashPerceptualHasher>();
            });
        }
    };
    static PerceptualHashRegistrar registrar;
}

#endif // FO_HAVE_OPENCV

} // namespace fo::core

// Always provide the registration function
#ifdef FO_HAVE_OPENCV
namespace fo::core {
static bool reg_perceptual_opencv_guard = true;
void register_perceptual_opencv() { (void)reg_perceptual_opencv_guard; }
} // namespace fo::core
#else
namespace fo::core {
void register_perceptual_opencv() {}
} // namespace fo::core
#endif
