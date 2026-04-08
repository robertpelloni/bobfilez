/// @file test_hex_editor.cpp
/// @brief Comprehensive tests for HexBuffer and DataInterpretation.

#include <gtest/gtest.h>
#include "fo/core/hex_editor_interface.hpp"
#include <filesystem>
#include <fstream>
#include <cstring>
#include <cmath>

using namespace fo::core;

class HexEditorTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto unique_id = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        test_dir = std::filesystem::temp_directory_path() / ("fo_test_hex_" + unique_id);
        std::filesystem::create_directories(test_dir);
    }
    void TearDown() override {
        if (std::filesystem::exists(test_dir)) {
            std::filesystem::remove_all(test_dir);
        }
    }

    void create_binary_file(const std::filesystem::path& path, const std::vector<uint8_t>& data) {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream ofs(path, std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
    }

    std::filesystem::path test_dir;
};

// ── DataInterpretation Tests ────────────────────────────────────────────

TEST_F(HexEditorTest, DataInterpretationNullInput) {
    auto r = DataInterpretation::from_bytes(nullptr, 0);
    EXPECT_EQ(r.as_uint8, 0);
    EXPECT_EQ(r.as_uint32_le, 0u);
}

TEST_F(HexEditorTest, DataInterpretationEmptyInput) {
    uint8_t dummy = 0;
    auto r = DataInterpretation::from_bytes(&dummy, 0);
    EXPECT_EQ(r.as_uint8, 0);
}

TEST_F(HexEditorTest, DataInterpretationSingleByte) {
    uint8_t data[] = {0x41}; // 'A'
    auto r = DataInterpretation::from_bytes(data, 1);
    EXPECT_EQ(r.as_uint8, 0x41);
    EXPECT_EQ(r.as_int8, 65);
    EXPECT_EQ(r.as_ascii, "A");
}

TEST_F(HexEditorTest, DataInterpretationUint16LE) {
    uint8_t data[] = {0x01, 0x00}; // 1 in LE
    auto r = DataInterpretation::from_bytes(data, 2);
    EXPECT_EQ(r.as_uint16_le, 1u);
    EXPECT_EQ(r.as_uint16_be, 256u); // 0x0100 = 256
}

TEST_F(HexEditorTest, DataInterpretationUint16BE) {
    uint8_t data[] = {0x00, 0x01};
    auto r = DataInterpretation::from_bytes(data, 2);
    EXPECT_EQ(r.as_uint16_be, 1u);
    EXPECT_EQ(r.as_uint16_le, 256u);
}

TEST_F(HexEditorTest, DataInterpretationUint32LE) {
    uint8_t data[] = {0x78, 0x56, 0x34, 0x12}; // 0x12345678 in LE
    auto r = DataInterpretation::from_bytes(data, 4);
    EXPECT_EQ(r.as_uint32_le, 0x12345678u);
    EXPECT_EQ(r.as_uint32_be, 0x78563412u);
}

TEST_F(HexEditorTest, DataInterpretationFloat32LE) {
    float val = 3.14f;
    uint8_t data[4];
    std::memcpy(data, &val, 4);
    auto r = DataInterpretation::from_bytes(data, 4);
    EXPECT_NEAR(r.as_float32_le, 3.14f, 0.001f);
}

TEST_F(HexEditorTest, DataInterpretationUint64LE) {
    uint8_t data[] = {0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01};
    auto r = DataInterpretation::from_bytes(data, 8);
    EXPECT_EQ(r.as_uint64_le, 0x0123456789ABCDEFull);
}

TEST_F(HexEditorTest, DataInterpretationFloat64LE) {
    double val = 2.718281828;
    uint8_t data[8];
    std::memcpy(data, &val, 8);
    auto r = DataInterpretation::from_bytes(data, 8);
    EXPECT_NEAR(r.as_float64_le, 2.718281828, 1e-6);
}

TEST_F(HexEditorTest, DataInterpretationHexOutput) {
    uint8_t data[] = {0xFF, 0x0A, 0x2B};
    auto r = DataInterpretation::from_bytes(data, 3);
    EXPECT_NE(r.as_hex.find("FF"), std::string::npos);
    EXPECT_NE(r.as_hex.find("0A"), std::string::npos);
}

TEST_F(HexEditorTest, DataInterpretationBinaryOutput) {
    uint8_t data[] = {0b10101010};
    auto r = DataInterpretation::from_bytes(data, 1);
    EXPECT_NE(r.as_binary.find("10101010"), std::string::npos);
}

TEST_F(HexEditorTest, DataInterpretationGuid16Bytes) {
    uint8_t data[16] = {0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89,
                        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    auto r = DataInterpretation::from_bytes(data, 16);
    EXPECT_FALSE(r.as_guid.empty());
    EXPECT_EQ(r.as_guid.front(), '{');
    EXPECT_EQ(r.as_guid.back(), '}');
}

TEST_F(HexEditorTest, DataInterpretationNoGuidIfLessThan16Bytes) {
    uint8_t data[8] = {0};
    auto r = DataInterpretation::from_bytes(data, 8);
    EXPECT_TRUE(r.as_guid.empty());
}

TEST_F(HexEditorTest, DataInterpretationNonPrintableASCII) {
    uint8_t data[] = {0x01, 0x02, 0x41, 0x42};
    auto r = DataInterpretation::from_bytes(data, 4);
    EXPECT_EQ(r.as_ascii.size(), 4u);
    EXPECT_EQ(r.as_ascii[0], '.'); // non-printable → dot
    EXPECT_EQ(r.as_ascii[2], 'A');
}

TEST_F(HexEditorTest, DataInterpretationUnixTimestamp) {
    // 2024-01-01 00:00:00 UTC = 1704067200 = 0x65922C80
    uint32_t ts = 1704067200;
    uint8_t data[4];
    std::memcpy(data, &ts, 4);
    auto r = DataInterpretation::from_bytes(data, 4);
    EXPECT_FALSE(r.as_unix_timestamp.empty());
}

// ── HexBuffer Tests ────────────────────────────────────────────────────

TEST_F(HexEditorTest, HexBufferOpenAndSize) {
    std::vector<uint8_t> content(1024, 0xAA);
    create_binary_file(test_dir / "test.bin", content);

    HexBuffer buf(test_dir / "test.bin", true);
    EXPECT_EQ(buf.file_size(), 1024);
    EXPECT_TRUE(buf.is_read_only());
    EXPECT_FALSE(buf.is_modified());
}

TEST_F(HexEditorTest, HexBufferReadBytes) {
    std::vector<uint8_t> content = {0x10, 0x20, 0x30, 0x40, 0x50};
    create_binary_file(test_dir / "read.bin", content);

    HexBuffer buf(test_dir / "read.bin", true);
    std::vector<uint8_t> out;
    EXPECT_TRUE(buf.read(0, 5, out));
    ASSERT_EQ(out.size(), 5u);
    EXPECT_EQ(out[0], 0x10);
    EXPECT_EQ(out[4], 0x50);
}

TEST_F(HexEditorTest, HexBufferReadPage) {
    std::vector<uint8_t> content(8192, 0xBB);
    create_binary_file(test_dir / "page.bin", content);

    HexBuffer buf(test_dir / "page.bin", true);
    auto page = buf.read_page(0, 4096);
    ASSERT_EQ(page.size(), 4096u);
    EXPECT_EQ(page[0], 0xBB);
}

TEST_F(HexEditorTest, HexBufferReadOutOfBoundsReturnsPartial) {
    std::vector<uint8_t> content = {0x01, 0x02, 0x03};
    create_binary_file(test_dir / "small.bin", content);

    HexBuffer buf(test_dir / "small.bin", true);
    std::vector<uint8_t> out;
    // Request 10 bytes but only 3 available
    EXPECT_TRUE(buf.read(0, 10, out));
    EXPECT_EQ(out.size(), 3u);
}

TEST_F(HexEditorTest, HexBufferReadPastEndFails) {
    std::vector<uint8_t> content = {0x01, 0x02};
    create_binary_file(test_dir / "tiny.bin", content);

    HexBuffer buf(test_dir / "tiny.bin", true);
    std::vector<uint8_t> out;
    EXPECT_FALSE(buf.read(10, 1, out)); // offset past end
}

TEST_F(HexEditorTest, HexBufferWriteAndReadBack) {
    std::vector<uint8_t> content = {0x00, 0x00, 0x00, 0x00};
    create_binary_file(test_dir / "writable.bin", content);

    HexBuffer buf(test_dir / "writable.bin", false); // writable
    EXPECT_TRUE(buf.write(1, {0xAA, 0xBB}));
    EXPECT_TRUE(buf.is_modified());

    std::vector<uint8_t> out;
    buf.read(0, 4, out);
    EXPECT_EQ(out[0], 0x00);
    EXPECT_EQ(out[1], 0xAA);
    EXPECT_EQ(out[2], 0xBB);
    EXPECT_EQ(out[3], 0x00);
}

TEST_F(HexEditorTest, HexBufferWriteFailsIfReadOnly) {
    std::vector<uint8_t> content = {0x00, 0x00};
    create_binary_file(test_dir / "ro.bin", content);

    HexBuffer buf(test_dir / "ro.bin", true);
    EXPECT_FALSE(buf.write(0, {0xFF}));
}

TEST_F(HexEditorTest, HexBufferUndoRedo) {
    std::vector<uint8_t> content = {0x00, 0x00, 0x00};
    create_binary_file(test_dir / "undo.bin", content);

    HexBuffer buf(test_dir / "undo.bin", false);
    buf.write(0, {0x11});
    buf.write(1, {0x22});
    EXPECT_TRUE(buf.is_modified());

    // Undo second write
    EXPECT_TRUE(buf.undo());
    std::vector<uint8_t> out;
    buf.read(0, 3, out);
    EXPECT_EQ(out[0], 0x11);
    EXPECT_EQ(out[1], 0x00); // undone
    EXPECT_EQ(out[2], 0x00);

    // Undo first write
    EXPECT_TRUE(buf.undo());
    buf.read(0, 3, out);
    EXPECT_EQ(out[0], 0x00);

    EXPECT_FALSE(buf.is_modified());

    // Redo
    EXPECT_TRUE(buf.redo());
    buf.read(0, 3, out);
    EXPECT_EQ(out[0], 0x11);
}

TEST_F(HexEditorTest, HexBufferInterpretAtOffset) {
    uint8_t data[4] = {0x78, 0x56, 0x34, 0x12};
    create_binary_file(test_dir / "interp.bin", {data, data + 4});

    HexBuffer buf(test_dir / "interp.bin", true);
    auto interp = buf.interpret(0);
    EXPECT_EQ(interp.as_uint32_le, 0x12345678u);
}

TEST_F(HexEditorTest, HexBufferEmptyFile) {
    create_binary_file(test_dir / "empty.bin", {});
    HexBuffer buf(test_dir / "empty.bin", true);
    EXPECT_EQ(buf.file_size(), 0);
}

TEST_F(HexEditorTest, HexBufferCanUndoRedoEmpty) {
    std::vector<uint8_t> content = {0x00};
    create_binary_file(test_dir / "undotest.bin", content);

    HexBuffer buf(test_dir / "undotest.bin", false);
    EXPECT_FALSE(buf.undo()); // nothing to undo
    EXPECT_FALSE(buf.redo()); // nothing to redo
}

TEST_F(HexEditorTest, HexBufferMultiplePatches) {
    std::vector<uint8_t> content(10, 0x00);
    create_binary_file(test_dir / "multi.bin", content);

    HexBuffer buf(test_dir / "multi.bin", false);
    buf.write(0, {0x01});
    buf.write(5, {0x55});
    buf.write(9, {0xFF});

    std::vector<uint8_t> out;
    buf.read(0, 10, out);
    EXPECT_EQ(out[0], 0x01);
    EXPECT_EQ(out[5], 0x55);
    EXPECT_EQ(out[9], 0xFF);
    EXPECT_EQ(out[4], 0x00); // unchanged
}

TEST_F(HexEditorTest, HexBufferOverlappingPatches) {
    std::vector<uint8_t> content(8, 0x00);
    create_binary_file(test_dir / "overlap.bin", content);

    HexBuffer buf(test_dir / "overlap.bin", false);
    buf.write(0, {0x11, 0x22, 0x33, 0x44});
    buf.write(2, {0xAA, 0xBB}); // Overlaps with first write

    std::vector<uint8_t> out;
    buf.read(0, 6, out);
    EXPECT_EQ(out[0], 0x11);
    EXPECT_EQ(out[1], 0x22);
    EXPECT_EQ(out[2], 0xAA); // Overwritten by second patch
    EXPECT_EQ(out[3], 0xBB); // Overwritten by second patch
}
