#pragma once
/// @file hex_editor_interface.hpp
/// @brief Hex viewer/editor backend for bobfilez.
///
/// Designed to support the features found in:
///   HxD           — fastest Windows hex editor, disk/RAM editing
///   010 Editor    — templates/scripts, structure parsing
///   ImHex         — modern open-source hex editor with patterns
///   Bless         — GTK hex editor with plugins
///   wxHexEditor   — cross-platform, large file support
///
/// Architecture:
///   HexBuffer     — virtual buffer that handles files of any size via mmap/paging.
///                   Only the visible window is loaded into RAM at a time.
///                   Edits are stored as a sparse overlay (patch list) so a 10GB
///                   file can be edited without copying it.
///   HexSearch     — searches for byte patterns, ASCII/UTF-8/UTF-16 strings,
///                   hex patterns with wildcards (e.g. "FF ?? AB")
///   DataInspector — interprets selected bytes as int8/16/32/64, float, double,
///                   little/big endian, GUID, timestamp, etc.
///   StructureParser — 010-Editor-style binary template parser

#include <string>
#include <vector>
#include <filesystem>
#include <cstdint>
#include <optional>
#include <functional>

namespace fo::core {

//────────────────────────── Data Inspector ───────────────────────────────────

/// Interprets up to 16 bytes in every common data type
struct DataInterpretation {
    // Signed integers
    int8_t   as_int8    = 0;
    int16_t  as_int16_le = 0, as_int16_be = 0;
    int32_t  as_int32_le = 0, as_int32_be = 0;
    int64_t  as_int64_le = 0, as_int64_be = 0;
    // Unsigned integers
    uint8_t  as_uint8   = 0;
    uint16_t as_uint16_le = 0, as_uint16_be = 0;
    uint32_t as_uint32_le = 0, as_uint32_be = 0;
    uint64_t as_uint64_le = 0, as_uint64_be = 0;
    // Floating point
    float    as_float32_le = 0.0f, as_float32_be = 0.0f;
    double   as_float64_le = 0.0,  as_float64_be = 0.0;
    // Text representations
    std::string as_ascii;          // Up to 16 printable ASCII chars
    std::string as_utf8;           // Decoded UTF-8 (may be shorter than 16)
    std::string as_utf16_le;       // Decoded UTF-16LE
    std::string as_utf16_be;       // Decoded UTF-16BE
    // Numeric helpers
    std::string as_hex;            // "FF 0A 2B ..."
    std::string as_binary;         // "11111111 00001010 ..."
    std::string as_octal;
    // Windows FILETIME (100ns since 1601-01-01)
    std::string as_filetime;
    // Unix timestamp
    std::string as_unix_timestamp;
    // GUID / UUID
    std::string as_guid;           // "{XXXXXXXX-XXXX-...}" if >= 16 bytes
    // CRC checks
    uint32_t crc32 = 0;
    uint16_t crc16 = 0;

    static DataInterpretation from_bytes(const uint8_t* data, size_t len);
};

//────────────────────────── Hex Search ───────────────────────────────────────

struct HexSearchOptions {
    enum class Mode {
        Hex,       // "FF ?? AB CD" — wildcards with ??
        ASCII,     // Plain ASCII string
        UTF16LE,   // Unicode string (little-endian)
        UTF16BE,
        Regex      // Regex over decoded text
    } mode = Mode::ASCII;

    std::string pattern;
    bool case_sensitive = true;
    bool wrap_around = true;
    bool regex_dot_all = false;
};

struct HexSearchResult {
    int64_t offset;   // Byte offset in file
    int length;       // Length of match in bytes
};

//────────────────────────── Hex Patch / Edit ─────────────────────────────────

/// A single edit operation in the sparse overlay
struct HexPatch {
    int64_t offset;
    std::vector<uint8_t> original; // For undo
    std::vector<uint8_t> patched;
};

//────────────────────────── Hex Buffer ───────────────────────────────────────

/// Virtual hex buffer — reads pages on-demand, stores edits as a patch overlay.
/// Supports files larger than RAM. The QML hex view calls read_page() to get
/// a view-window of bytes, and apply_patch() to edit them.
class HexBuffer {
public:
    explicit HexBuffer(const std::filesystem::path& path, bool read_only = true);
    ~HexBuffer();

    int64_t file_size() const;
    bool is_read_only() const { return read_only_; }
    bool is_modified() const { return !patches_.empty(); }

    /// Read `length` bytes from `offset` into `out`. Applies patches on top.
    bool read(int64_t offset, size_t length, std::vector<uint8_t>& out) const;

    /// Write bytes at offset (adds to patch list, doesn't touch disk yet)
    bool write(int64_t offset, const std::vector<uint8_t>& data);

    /// Insert bytes at offset (shifts all following bytes)
    bool insert(int64_t offset, const std::vector<uint8_t>& data);

    /// Delete bytes at offset
    bool remove(int64_t offset, size_t count);

    /// Save changes back to file (or to a new file)
    bool save(const std::filesystem::path& dest = {});

    /// Undo last edit
    bool undo();
    bool redo();
    bool can_undo() const;
    bool can_redo() const;

    /// Search forward/backward from `from_offset`
    std::optional<HexSearchResult> search(const HexSearchOptions& opts, int64_t from_offset = 0) const;
    std::vector<HexSearchResult> search_all(const HexSearchOptions& opts) const;

    /// Interpret bytes at offset
    DataInterpretation interpret(int64_t offset) const;

    /// Page access for QML rendering (returns `page_size` bytes)
    std::vector<uint8_t> read_page(int64_t page_offset, int page_size = 4096) const;

    const std::filesystem::path& path() const { return path_; }

private:
    std::filesystem::path path_;
    bool read_only_;
    int64_t file_size_ = 0;
    mutable void* mmap_handle_ = nullptr;
    mutable void* file_handle_ = nullptr;
    std::vector<HexPatch> patches_;
    std::vector<HexPatch> redo_stack_;

    void ensure_mapped() const;
    void apply_patches(int64_t offset, std::vector<uint8_t>& data) const;
};

} // namespace fo::core
