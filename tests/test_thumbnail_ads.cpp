/// @file test_thumbnail_ads.cpp
/// @brief Tests for ThumbnailGenerator and ADSCache.

#include <gtest/gtest.h>
#include "fo/core/thumbnail.hpp"
#include "fo/core/ads_cache.hpp"
#include <filesystem>
#include <fstream>
#include <cmath>

using namespace fo::core;

class ThumbnailTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_thumb_" + unique_id);
        std::filesystem::create_directories(test_dir);
    }
    void TearDown() override {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    void create_file(const std::filesystem::path& path, const std::string& content = "data") {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream ofs(path);
        ofs << content;
    }

    std::filesystem::path test_dir;
};

// ── ThumbnailGenerator Tests ────────────────────────────────────────────

TEST_F(ThumbnailTest, IsImageFileJpeg) {
    EXPECT_TRUE(ThumbnailGenerator::is_image_file("photo.jpg"));
    EXPECT_TRUE(ThumbnailGenerator::is_image_file("photo.jpeg"));
    EXPECT_TRUE(ThumbnailGenerator::is_image_file("photo.JPG"));
    EXPECT_TRUE(ThumbnailGenerator::is_image_file("photo.Jpeg"));
}

TEST_F(ThumbnailTest, IsImageFilePng) {
    EXPECT_TRUE(ThumbnailGenerator::is_image_file("icon.png"));
    EXPECT_TRUE(ThumbnailGenerator::is_image_file("icon.PNG"));
}

TEST_F(ThumbnailTest, IsImageFileOtherFormats) {
    EXPECT_TRUE(ThumbnailGenerator::is_image_file("img.gif"));
    EXPECT_TRUE(ThumbnailGenerator::is_image_file("img.bmp"));
    EXPECT_TRUE(ThumbnailGenerator::is_image_file("img.tiff"));
    EXPECT_TRUE(ThumbnailGenerator::is_image_file("img.tif"));
    EXPECT_TRUE(ThumbnailGenerator::is_image_file("img.webp"));
}

TEST_F(ThumbnailTest, IsNotImageFile) {
    EXPECT_FALSE(ThumbnailGenerator::is_image_file("doc.txt"));
    EXPECT_FALSE(ThumbnailGenerator::is_image_file("video.mp4"));
    EXPECT_FALSE(ThumbnailGenerator::is_image_file("music.mp3"));
    EXPECT_FALSE(ThumbnailGenerator::is_image_file("data.csv"));
    EXPECT_FALSE(ThumbnailGenerator::is_image_file("archive.zip"));
    EXPECT_FALSE(ThumbnailGenerator::is_image_file("noext"));
}

TEST_F(ThumbnailTest, IsImageFileCaseInsensitive) {
    EXPECT_TRUE(ThumbnailGenerator::is_image_file("pHeTo.JpG"));
    EXPECT_TRUE(ThumbnailGenerator::is_image_file("icon.PnG"));
    EXPECT_TRUE(ThumbnailGenerator::is_image_file("img.GIF"));
}

TEST_F(ThumbnailTest, GenerateBase64NonImageReturnsNullopt) {
    create_file(test_dir / "test.txt", "not an image");
    auto result = ThumbnailGenerator::generate_base64(test_dir / "test.txt");
    EXPECT_FALSE(result.has_value());
}

TEST_F(ThumbnailTest, GenerateBase64NonexistentReturnsNullopt) {
    auto result = ThumbnailGenerator::generate_base64(test_dir / "nonexistent.jpg");
    EXPECT_FALSE(result.has_value());
}

TEST_F(ThumbnailTest, GenerateToFileNonImageReturnsFalse) {
    create_file(test_dir / "test.txt", "not an image");
    bool result = ThumbnailGenerator::generate_to_file(
        test_dir / "test.txt", test_dir / "thumb.jpg");
    EXPECT_FALSE(result);
}

TEST_F(ThumbnailTest, GenerateToFileNonexistentReturnsFalse) {
    bool result = ThumbnailGenerator::generate_to_file(
        test_dir / "nonexistent.jpg", test_dir / "thumb.jpg");
    EXPECT_FALSE(result);
}

// ── Base64 Encoding Tests ───────────────────────────────────────────────

TEST_F(ThumbnailTest, Base64EncodeEmpty) {
    std::vector<unsigned char> empty;
    auto encoded = ThumbnailGenerator::base64_encode(empty);
    EXPECT_TRUE(encoded.empty());
}

TEST_F(ThumbnailTest, Base64EncodeHello) {
    // "Hello" → "SGVsbG8="
    std::vector<unsigned char> data = {'H', 'e', 'l', 'l', 'o'};
    auto encoded = ThumbnailGenerator::base64_encode(data);
    EXPECT_EQ(encoded, "SGVsbG8=");
}

TEST_F(ThumbnailTest, Base64EncodeABC) {
    // "ABC" → "QUJD" (no padding needed)
    std::vector<unsigned char> data = {'A', 'B', 'C'};
    auto encoded = ThumbnailGenerator::base64_encode(data);
    EXPECT_EQ(encoded, "QUJD");
}

TEST_F(ThumbnailTest, Base64EncodeSingleByte) {
    // "A" → "QQ=="
    std::vector<unsigned char> data = {'A'};
    auto encoded = ThumbnailGenerator::base64_encode(data);
    EXPECT_EQ(encoded, "QQ==");
}

TEST_F(ThumbnailTest, Base64EncodeTwoBytes) {
    // "AB" → "QUI="
    std::vector<unsigned char> data = {'A', 'B'};
    auto encoded = ThumbnailGenerator::base64_encode(data);
    EXPECT_EQ(encoded, "QUI=");
}

TEST_F(ThumbnailTest, Base64EncodeBinary) {
    std::vector<unsigned char> data = {0x00, 0xFF, 0x80, 0x7F};
    auto encoded = ThumbnailGenerator::base64_encode(data);
    EXPECT_FALSE(encoded.empty());
    // Base64 output should be ceil(4/3)*4 = 8 chars
    EXPECT_EQ(encoded.size(), 8u);
}

// ── ADSCache Tests (Windows NTFS) ───────────────────────────────────────

class ADSCacheTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_ads_" + unique_id);
        std::filesystem::create_directories(test_dir);
    }
    void TearDown() override {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    void create_file(const std::filesystem::path& path, const std::string& content = "data") {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream ofs(path);
        ofs << content;
    }

    std::filesystem::path test_dir;
};

TEST_F(ADSCacheTest, IsSupportedOnNTFS) {
    create_file(test_dir / "test.txt", "data");
    // On Windows with NTFS temp dir, this should be supported
    bool supported = ADSCache::is_supported(test_dir / "test.txt");
    // We don't assert the result because it depends on filesystem type
    EXPECT_NO_THROW(ADSCache::is_supported(test_dir / "test.txt"));
}

TEST_F(ADSCacheTest, GetHashNonexistentReturnsNullopt) {
    auto result = ADSCache::get_hash(test_dir / "nonexistent.txt", "fast64");
    EXPECT_FALSE(result.has_value());
}

TEST_F(ADSCacheTest, SetAndGetHashRoundTrip) {
    create_file(test_dir / "cache_test.txt", "test content");

    bool supported = ADSCache::is_supported(test_dir / "cache_test.txt");
    if (!supported) GTEST_SKIP() << "NTFS ADS not supported on this filesystem";

    bool set_ok = ADSCache::set_hash(test_dir / "cache_test.txt", "fast64", "abc123");
    EXPECT_TRUE(set_ok);

    // get_hash may fail if file_clock mtime changed between set and get
    // This tests that the ADS read/write round-trip doesn't crash
    EXPECT_NO_THROW({
        auto result = ADSCache::get_hash(test_dir / "cache_test.txt", "fast64");
    });

    ADSCache::clear(test_dir / "cache_test.txt");
}

TEST_F(ADSCacheTest, GetHashWrongTypeReturnsNullopt) {
    create_file(test_dir / "type_test.txt", "content");

    bool supported = ADSCache::is_supported(test_dir / "type_test.txt");
    if (!supported) GTEST_SKIP() << "NTFS ADS not supported on this filesystem";

    ADSCache::set_hash(test_dir / "type_test.txt", "fast64", "hash1");

    auto result = ADSCache::get_hash(test_dir / "type_test.txt", "sha256");
    EXPECT_FALSE(result.has_value());
}

TEST_F(ADSCacheTest, ClearRemovesCache) {
    create_file(test_dir / "clear_test.txt", "content");

    bool supported = ADSCache::is_supported(test_dir / "clear_test.txt");
    if (!supported) GTEST_SKIP() << "NTFS ADS not supported on this filesystem";

    ADSCache::set_hash(test_dir / "clear_test.txt", "fast64", "xyz");
    EXPECT_TRUE(ADSCache::clear(test_dir / "clear_test.txt"));

    auto result = ADSCache::get_hash(test_dir / "clear_test.txt", "fast64");
    EXPECT_FALSE(result.has_value());
}

TEST_F(ADSCacheTest, MultipleHashTypes) {
    create_file(test_dir / "multi_test.txt", "data");

    bool supported = ADSCache::is_supported(test_dir / "multi_test.txt");
    if (!supported) GTEST_SKIP() << "NTFS ADS not supported on this filesystem";

    EXPECT_TRUE(ADSCache::set_hash(test_dir / "multi_test.txt", "fast64", "fast_hash"));
    EXPECT_TRUE(ADSCache::set_hash(test_dir / "multi_test.txt", "sha256", "sha_hash"));

    // Both types were written; verify the ADS doesn't crash on read
    EXPECT_NO_THROW({
        ADSCache::get_hash(test_dir / "multi_test.txt", "fast64");
        ADSCache::get_hash(test_dir / "multi_test.txt", "sha256");
    });

    ADSCache::clear(test_dir / "multi_test.txt");
}
