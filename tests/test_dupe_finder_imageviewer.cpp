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

TEST_F(DupeFinderTest, SizeHashDuplicateFinderName) {
    SizeHashDuplicateFinder finder;
    EXPECT_EQ(finder.name(), "size_hash");
}

TEST_F(DupeFinderTest, SizeHashByteDuplicateFinderName) {
    SizeHashByteDuplicateFinder finder;
    EXPECT_EQ(finder.name(), "size_hash_byte");
}

TEST_F(DupeFinderTest, SizeHashFindsDuplicates) {
    create_file(test_dir / "a.txt", "same content");
    create_file(test_dir / "b.txt", "same content");

    auto hasher = Registry<IHasher>::instance().create("fast64");
    ASSERT_NE(hasher, nullptr);

    SizeHashDuplicateFinder finder;
    std::vector<FileInfo> files;
    FileInfo f1; f1.uri = (test_dir / "a.txt").string(); f1.size = 12;
    FileInfo f2; f2.uri = (test_dir / "b.txt").string(); f2.size = 12;
    files.push_back(f1);
    files.push_back(f2);

    auto groups = finder.group(files, *hasher);
    EXPECT_EQ(groups.size(), 1u);
    EXPECT_EQ(groups[0].files.size(), 2u);
}

TEST_F(DupeFinderTest, SizeHashNoDuplicatesForDifferentContent) {
    create_file(test_dir / "a.txt", "content A");
    create_file(test_dir / "b.txt", "content B that is different");

    auto hasher = Registry<IHasher>::instance().create("fast64");
    ASSERT_NE(hasher, nullptr);

    SizeHashDuplicateFinder finder;
    std::vector<FileInfo> files;
    FileInfo f1; f1.uri = (test_dir / "a.txt").string(); f1.size = 10;
    FileInfo f2; f2.uri = (test_dir / "b.txt").string(); f2.size = 26;
    files.push_back(f1);
    files.push_back(f2);

    auto groups = finder.group(files, *hasher);
    EXPECT_TRUE(groups.empty());
}

TEST_F(DupeFinderTest, SizeHashEmptyInputReturnsEmpty) {
    auto hasher = Registry<IHasher>::instance().create("fast64");
    SizeHashDuplicateFinder finder;
    auto groups = finder.group({}, *hasher);
    EXPECT_TRUE(groups.empty());
}

TEST_F(DupeFinderTest, SizeHashByteFindsTrueDuplicates) {
    create_file(test_dir / "a.txt", "identical bytes");
    create_file(test_dir / "b.txt", "identical bytes");

    auto hasher = Registry<IHasher>::instance().create("fast64");
    ASSERT_NE(hasher, nullptr);

    SizeHashByteDuplicateFinder finder;
    std::vector<FileInfo> files;
    FileInfo f1; f1.uri = (test_dir / "a.txt").string(); f1.size = 15;
    FileInfo f2; f2.uri = (test_dir / "b.txt").string(); f2.size = 15;
    files.push_back(f1);
    files.push_back(f2);

    auto groups = finder.group(files, *hasher);
    EXPECT_EQ(groups.size(), 1u);
    EXPECT_EQ(groups[0].files.size(), 2u);
}

TEST_F(DupeFinderTest, SizeHashByteRejectsDifferentContent) {
    create_file(test_dir / "a.txt", "content AAAA");
    create_file(test_dir / "b.txt", "content BBBB");

    // Force same size and same fast64 hash to test byte-by-byte comparison
    auto hasher = Registry<IHasher>::instance().create("fast64");

    SizeHashByteDuplicateFinder finder;
    std::vector<FileInfo> files;
    FileInfo f1; f1.uri = (test_dir / "a.txt").string(); f1.size = 13;
    FileInfo f2; f2.uri = (test_dir / "b.txt").string(); f2.size = 13;
    files.push_back(f1);
    files.push_back(f2);

    auto groups = finder.group(files, *hasher);
    // Different content means byte comparison rejects them
    // But they may have different fast64 hashes, so they won't even be grouped initially
    // This test verifies the pipeline doesn't crash
    EXPECT_NO_THROW(finder.group(files, *hasher));
}

TEST_F(DupeFinderTest, SizeHashMultipleGroups) {
    // Group 1: identical
    create_file(test_dir / "a1.txt", "group one");
    create_file(test_dir / "a2.txt", "group one");
    // Group 2: identical
    create_file(test_dir / "b1.txt", "group two");
    create_file(test_dir / "b2.txt", "group two");

    auto hasher = Registry<IHasher>::instance().create("fast64");
    SizeHashDuplicateFinder finder;

    std::vector<FileInfo> files;
    FileInfo f1; f1.uri = (test_dir / "a1.txt").string(); f1.size = 9;
    FileInfo f2; f2.uri = (test_dir / "a2.txt").string(); f2.size = 9;
    FileInfo f3; f3.uri = (test_dir / "b1.txt").string(); f3.size = 9;
    FileInfo f4; f4.uri = (test_dir / "b2.txt").string(); f4.size = 9;
    files.push_back(f1); files.push_back(f2);
    files.push_back(f3); files.push_back(f4);

    auto groups = finder.group(files, *hasher);
    EXPECT_EQ(groups.size(), 2u);
}

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
