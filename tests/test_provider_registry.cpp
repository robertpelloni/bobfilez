// Provider registry: the explicit-registration contract that keeps
// providers from being linker-dropped out of fo_core.lib.
#include <gtest/gtest.h>

#include "fo/core/audio_fingerprint_interface.hpp"
#include "fo/core/classification_interface.hpp"
#include "fo/core/interfaces.hpp"
#include "fo/core/lint_interface.hpp"
#include "fo/core/ocr_interface.hpp"
#include "fo/core/perceptual_hash_interface.hpp"
#include "fo/core/provider_registration.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/video_hash_interface.hpp"

#include <algorithm>

namespace {

bool contains(const std::vector<std::string>& names, const std::string& want) {
    return std::find(names.begin(), names.end(), want) != names.end();
}

struct RegistryFixture : ::testing::Test {
    static void SetUpTestSuite() { fo::core::register_all_providers(); }
};

TEST_F(RegistryFixture, CoreProvidersRegistered) {
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IFileScanner>::instance().names(), "std"));
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IHasher>::instance().names(), "fast64"));
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IHasher>::instance().names(), "sha256"));
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::ILinter>::instance().names(), "std"));
    // "dhash" is OpenCV-backed; asserted under FO_HAVE_OPENCV below.
}

#ifdef FO_HAVE_OPENCV
TEST_F(RegistryFixture, PerceptualHashersRegisteredWhenOpenCVPresent) {
    // Pins the explicit register_perceptual_opencv() path: a TU-local static
    // registrar was linker-dropped in the JUCE demo tree (dhash missing).
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IPerceptualHasher>::instance().names(), "dhash"));
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IPerceptualHasher>::instance().names(), "phash"));
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IPerceptualHasher>::instance().names(), "ahash"));
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IPerceptualHasher>::instance().names(), "opencv"));
}
#endif

#ifdef FO_HAVE_FFMPEG
TEST_F(RegistryFixture, VideoHasherRegisteredWhenFFmpegPresent) {
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IVideoHasher>::instance().names(), "ffmpeg"));
}
#endif

#if defined(FO_HAVE_CHROMAPRINT) && defined(FO_HAVE_FFMPEG)
TEST_F(RegistryFixture, AudioFingerprinterRegisteredWhenChromaprintPresent) {
    EXPECT_TRUE(contains(fo::core::Registry<fo::core::IAudioFingerprinter>::instance().names(), "chromaprint"));
}
#endif

TEST_F(RegistryFixture, UnknownProviderCreatesNull) {
    EXPECT_EQ(fo::core::Registry<fo::core::IFileScanner>::instance().create("does-not-exist"), nullptr);
}

} // namespace
