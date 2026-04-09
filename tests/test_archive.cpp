/// @file test_archive.cpp
/// @brief Tests for AdvancedArchiveManager structs and interface.

#include <gtest/gtest.h>
#include "fo/core/enhanced_fileops_interface.hpp"
#include "fo/core/fileops_interface.hpp"
#include <filesystem>
#include <fstream>

using namespace fo::core;

// ── ArchiveBrowserEntry Struct Tests ─────────────────────────────────────

TEST(ArchiveBrowserEntryTest, DefaultInitialization) {
    ArchiveBrowserEntry entry;
    EXPECT_EQ(entry.path, "");
    EXPECT_EQ(entry.size, 0);
    EXPECT_EQ(entry.compressed, 0);
    EXPECT_FALSE(entry.is_dir);
    EXPECT_EQ(entry.crc, "");
    EXPECT_FALSE(entry.encrypted);
    EXPECT_EQ(entry.compression_method, "");
    EXPECT_EQ(entry.comment, "");
}

TEST(ArchiveBrowserEntryTest, PopulateFields) {
    ArchiveBrowserEntry entry;
    entry.path = "docs/report.pdf";
    entry.size = 1048576;
    entry.compressed = 512000;
    entry.is_dir = false;
    entry.crc = "ABCD1234";
    entry.encrypted = true;
    entry.compression_method = "LZMA2";

    EXPECT_EQ(entry.path, "docs/report.pdf");
    EXPECT_EQ(entry.size, 1048576);
    EXPECT_EQ(entry.compressed, 512000);
    EXPECT_FALSE(entry.is_dir);
    EXPECT_EQ(entry.crc, "ABCD1234");
    EXPECT_TRUE(entry.encrypted);
    EXPECT_EQ(entry.compression_method, "LZMA2");
}

// ── ArchiveCreateOptions Tests ───────────────────────────────────────────

TEST(ArchiveCreateOptionsTest, DefaultValues) {
    ArchiveCreateOptions opts;
    EXPECT_EQ(opts.format, ArchiveFormat::SevenZip);
    EXPECT_EQ(opts.compression_level, 5);
    EXPECT_EQ(opts.compression_method, "");
    EXPECT_FALSE(opts.encrypt_contents);
    EXPECT_FALSE(opts.encrypt_filenames);
    EXPECT_EQ(opts.password, "");
    EXPECT_TRUE(opts.solid_mode);
    EXPECT_TRUE(opts.include_root_dir);
    EXPECT_FALSE(opts.self_extracting);
    EXPECT_FALSE(opts.split_volumes);
    EXPECT_EQ(opts.volume_size_bytes, 0);
    EXPECT_EQ(opts.num_threads, 0);
    EXPECT_TRUE(opts.exclude_patterns.empty());
}

TEST(ArchiveCreateOptionsTest, CustomValues) {
    ArchiveCreateOptions opts;
    opts.format = ArchiveFormat::Zip;
    opts.compression_level = 9;
    opts.compression_method = "Deflate";
    opts.encrypt_contents = true;
    opts.password = "secret123";
    opts.num_threads = 4;
    opts.exclude_patterns = {"*.tmp", "*.log"};

    EXPECT_EQ(opts.format, ArchiveFormat::Zip);
    EXPECT_EQ(opts.compression_level, 9);
    EXPECT_EQ(opts.compression_method, "Deflate");
    EXPECT_TRUE(opts.encrypt_contents);
    EXPECT_EQ(opts.password, "secret123");
    EXPECT_EQ(opts.num_threads, 4);
    EXPECT_EQ(opts.exclude_patterns.size(), 2u);
}

// ── ArchiveExtractOptions Tests ──────────────────────────────────────────

TEST(ArchiveExtractOptionsTest, DefaultValues) {
    ArchiveExtractOptions opts;
    EXPECT_TRUE(opts.dest_dir.empty());
    EXPECT_FALSE(opts.overwrite);
    EXPECT_EQ(opts.password, "");
    EXPECT_TRUE(opts.include_patterns.empty());
    EXPECT_TRUE(opts.keep_structure);
    EXPECT_FALSE(opts.verify_after);
}

TEST(ArchiveExtractOptionsTest, CustomValues) {
    ArchiveExtractOptions opts;
    opts.dest_dir = "/tmp/extract";
    opts.overwrite = true;
    opts.password = "mypass";
    opts.include_patterns = {"*.pdf", "*.docx"};
    opts.verify_after = true;

    EXPECT_EQ(opts.dest_dir, "/tmp/extract");
    EXPECT_TRUE(opts.overwrite);
    EXPECT_TRUE(opts.verify_after);
    EXPECT_EQ(opts.include_patterns.size(), 2u);
}

// ── TestResult Struct Tests ──────────────────────────────────────────────

TEST(ArchiveTestResultTest, DefaultIsOk) {
    AdvancedArchiveManager::TestResult result;
    EXPECT_TRUE(result.ok);
    EXPECT_TRUE(result.bad_entries.empty());
    EXPECT_EQ(result.error, "");
}

TEST(ArchiveTestResultTest, FailureState) {
    AdvancedArchiveManager::TestResult result;
    result.ok = false;
    result.error = "CRC mismatch";
    result.bad_entries = {"corrupt_file.bin", "broken_data.dat"};

    EXPECT_FALSE(result.ok);
    EXPECT_EQ(result.error, "CRC mismatch");
    EXPECT_EQ(result.bad_entries.size(), 2u);
}

// ── BenchmarkResult Struct Tests ─────────────────────────────────────────

TEST(ArchiveBenchmarkResultTest, Initialization) {
    AdvancedArchiveManager::BenchmarkResult br{};
    EXPECT_EQ(br.algorithm, "");
    EXPECT_EQ(br.level, 0);
    EXPECT_DOUBLE_EQ(br.ratio, 0.0);
    EXPECT_DOUBLE_EQ(br.compress_sec, 0.0);
    EXPECT_DOUBLE_EQ(br.decompress_sec, 0.0);
}

TEST(ArchiveBenchmarkResultTest, PopulateAndVerify) {
    AdvancedArchiveManager::BenchmarkResult br;
    br.algorithm = "LZMA2";
    br.level = 9;
    br.compressed_size = 256000;
    br.ratio = 0.2441;
    br.compress_sec = 2.5;
    br.decompress_sec = 0.3;
    br.compress_speed = 12.5;
    br.decompress_speed = 50.0;

    EXPECT_EQ(br.algorithm, "LZMA2");
    EXPECT_EQ(br.level, 9);
    EXPECT_NEAR(br.ratio, 0.2441, 0.001);
    EXPECT_DOUBLE_EQ(br.compress_sec, 2.5);
    EXPECT_DOUBLE_EQ(br.decompress_sec, 0.3);
}

// ── AdvancedArchiveManager Browse on nonexistent file ────────────────────

TEST(ArchiveManagerTest, BrowseNonexistentReturnsEmpty) {
    AdvancedArchiveManager mgr;
    auto entries = mgr.browse("/nonexistent/archive.7z");
    EXPECT_TRUE(entries.empty());
}

TEST(ArchiveManagerTest, TestNonexistentArchiveDoesNotCrash) {
    AdvancedArchiveManager mgr;
    EXPECT_NO_THROW(mgr.test("/nonexistent/archive.7z"));
}

TEST(ArchiveManagerTest, ExtractEntriesNonexistentDoesNotCrash) {
    AdvancedArchiveManager mgr;
    ArchiveExtractOptions opts;
    opts.dest_dir = "/tmp/out";
    EXPECT_NO_THROW(mgr.extract_entries("/nonexistent/archive.7z", {"file.txt"}, opts));
}

TEST(ArchiveManagerTest, DeleteEntriesNonexistentDoesNotCrash) {
    AdvancedArchiveManager mgr;
    ArchiveCreateOptions opts;
    EXPECT_NO_THROW(mgr.delete_entries("/nonexistent/archive.7z", {"file.txt"}, opts));
}

TEST(ArchiveManagerTest, RenameEntryNonexistentDoesNotCrash) {
    AdvancedArchiveManager mgr;
    EXPECT_NO_THROW(mgr.rename_entry("/nonexistent/archive.7z", "old.txt", "new.txt"));
}

TEST(ArchiveManagerTest, SearchInArchivesReturnsEmpty) {
    AdvancedArchiveManager mgr;
    auto results = mgr.search_in_archives({"/no/archive.zip"}, "*.pdf");
    EXPECT_TRUE(results.empty());
}

TEST(ArchiveManagerTest, BenchmarkReturnsEmpty) {
    AdvancedArchiveManager mgr;
    auto results = mgr.benchmark({"/no/file.bin"}, {"LZMA2"});
    EXPECT_TRUE(results.empty());
}

TEST(ArchiveInfoTest, DefaultInitialization) {
    AdvancedArchiveManager::ArchiveInfo info{};
    EXPECT_EQ(info.entry_count, 0);
    EXPECT_EQ(info.total_size, 0);
    EXPECT_EQ(info.uncompressed_size, 0);
    EXPECT_DOUBLE_EQ(info.ratio, 0.0);
    EXPECT_FALSE(info.encrypted);
    EXPECT_FALSE(info.solid);
    EXPECT_EQ(info.compression_method, "");
}

TEST(ArchiveInfoTest, PopulateFields) {
    AdvancedArchiveManager::ArchiveInfo info;
    info.entry_count = 42;
    info.total_size = 3 * 1024 * 1024;
    info.uncompressed_size = 10 * 1024 * 1024;
    info.encrypted = true;
    info.solid = true;
    info.compression_method = "LZMA2";

    EXPECT_EQ(info.entry_count, 42);
    EXPECT_TRUE(info.encrypted);
    EXPECT_EQ(info.compression_method, "LZMA2");
}

TEST(ArchiveManagerTest, RepairZipNonexistentFails) {
    AdvancedArchiveManager mgr;
    bool ok = mgr.repair_zip("/no/zip.zip", "/output/repaired.zip");
    EXPECT_FALSE(ok);
}

TEST(ArchiveManagerTest, MergeVolumesReturnsFalse) {
    AdvancedArchiveManager mgr;
    bool ok = mgr.merge_volumes({"/v1.7z.001", "/v1.7z.002"}, "/merged.7z");
    EXPECT_FALSE(ok);
}
