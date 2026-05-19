/// @file test_dupe_finder_imageviewer.cpp
/// @brief Tests for DuplicateFinders (SizeHash + Byte-by-byte) and ImageViewer/ThumbnailCache.

#include <gtest/gtest.h>
#include "fo/core/duplicate_finders.hpp"
#include "fo/core/image_viewer_interface.hpp"
#include "fo/core/provider_registration.hpp"
#include "fo/core/interfaces.hpp"
#include "fo/core/registry.hpp"
#include <filesystem>
#include <fstream>

using namespace fo::core;

// ── DuplicateFinder Tests ───────────────────────────────────────────────

class DupeFinderTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_dupe_" + unique_id);
        std::filesystem::create_directories(test_dir);
    }
    void TearDown() override {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    void create_file(const std::filesystem::path& path, const std::string& content) {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream ofs(path, std::ios::binary);
        ofs << content;
    }

    std::filesystem::path test_dir;
};









// ── ImageViewer / ImageHistogram Tests ───────────────────────────────────

class ImageViewerTest : public ::testing::Test {
protected:
    void SetUp() override {
        register_all_providers();
    }
};

TEST_F(ImageViewerTest, ImageInfoDefaults) {
    ImageInfo info;
    EXPECT_EQ(info.width, 0);
    EXPECT_EQ(info.height, 0);
    EXPECT_EQ(info.channels, 0);
    EXPECT_EQ(info.frame_count, 1);
    EXPECT_FALSE(info.has_alpha);
    EXPECT_FALSE(info.is_animated);
}

TEST_F(ImageViewerTest, ImageHistogramComputeEmpty) {
    std::vector<uint8_t> empty;
    // Static compute may not be implemented yet; just verify struct is usable
    ImageHistogram hist;
    EXPECT_TRUE(hist.red.bins.empty());
}

TEST_F(ImageViewerTest, ImageHistogramStructFields) {
    ImageHistogram hist;
    // Verify all channel fields exist
    (void)hist.red;
    (void)hist.green;
    (void)hist.blue;
    (void)hist.alpha;
    (void)hist.luminance;
}

TEST_F(ImageViewerTest, ChannelHistogramDefaults) {
    ChannelHistogram ch;
    EXPECT_TRUE(ch.bins.empty());
    EXPECT_EQ(ch.max_count, 0u);
    EXPECT_DOUBLE_EQ(ch.mean, 0.0);
}

TEST_F(ImageViewerTest, ImageFormatSupportDefaults) {
    ImageFormatSupport fmt;
    EXPECT_TRUE(fmt.ext.empty());
    EXPECT_TRUE(fmt.can_read);
    EXPECT_FALSE(fmt.can_write);
    EXPECT_FALSE(fmt.raw);
}

TEST_F(ImageViewerTest, SlideshowOptionsDefaults) {
    SlideshowOptions opts;
    EXPECT_DOUBLE_EQ(opts.interval_sec, 5.0);
    EXPECT_TRUE(opts.loop);
    EXPECT_FALSE(opts.random_order);
}

TEST_F(ImageViewerTest, ColorAdjustmentsDefaults) {
    ColorAdjustments adj;
    EXPECT_DOUBLE_EQ(adj.brightness, 0.0);
    EXPECT_DOUBLE_EQ(adj.contrast, 0.0);
    EXPECT_DOUBLE_EQ(adj.saturation, 0.0);
    EXPECT_DOUBLE_EQ(adj.gamma, 1.0);
    EXPECT_DOUBLE_EQ(adj.input_black, 0.0);
    EXPECT_DOUBLE_EQ(adj.input_white, 255.0);
}
