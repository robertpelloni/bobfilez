/// @file hex_editor.cpp
/// @brief Implementation of the memory-mapped HexBuffer and Data Inspector.

#include "fo/core/hex_editor_interface.hpp"
#include <stdexcept>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace fo::core {

//─────────────────────────── Data Interpretation ───────────────────────────────

DataInterpretation DataInterpretation::from_bytes(const uint8_t* data, size_t len) {
    DataInterpretation r;
    if (!data || len == 0) return r;

    // Helper to format hex/binary strings
    std::ostringstream hex_oss;
    std::string bin_str;
    for (size_t i = 0; i < std::min(len, (size_t)16); ++i) {
        hex_oss << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (int)data[i] << " ";
        if (data[i] >= 32 && data[i] < 127) r.as_ascii += static_cast<char>(data[i]);
        else r.as_ascii += '.';

        for (int b = 7; b >= 0; --b) bin_str += ((data[i] >> b) & 1) ? '1' : '0';
        bin_str += " ";
    }
    r.as_hex = hex_oss.str();
    r.as_binary = bin_str;

    if (len >= 1) {
        r.as_int8 = static_cast<int8_t>(data[0]);
        r.as_uint8 = static_cast<uint8_t>(data[0]);
    }
    if (len >= 2) {
        r.as_uint16_le = (data[1] << 8) | data[0];
        r.as_uint16_be = (data[0] << 8) | data[1];
        r.as_int16_le = static_cast<int16_t>(r.as_uint16_le);
        r.as_int16_be = static_cast<int16_t>(r.as_uint16_be);
    }
    if (len >= 4) {
        r.as_uint32_le = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
        r.as_uint32_be = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
        r.as_int32_le = static_cast<int32_t>(r.as_uint32_le);
        r.as_int32_be = static_cast<int32_t>(r.as_uint32_be);

        // IEEE 754 float
        std::memcpy(&r.as_float32_le, &r.as_uint32_le, 4);
        std::memcpy(&r.as_float32_be, &r.as_uint32_be, 4);
    }
    if (len >= 8) {
        r.as_uint64_le = ((uint64_t)data[7] << 56) | ((uint64_t)data[6] << 48) |
                         ((uint64_t)data[5] << 40) | ((uint64_t)data[4] << 32) |
                         ((uint64_t)data[3] << 24) | ((uint64_t)data[2] << 16) |
                         ((uint64_t)data[1] << 8)  | data[0];
        r.as_uint64_be = ((uint64_t)data[0] << 56) | ((uint64_t)data[1] << 48) |
                         ((uint64_t)data[2] << 40) | ((uint64_t)data[3] << 32) |
                         ((uint64_t)data[4] << 24) | ((uint64_t)data[5] << 16) |
                         ((uint64_t)data[6] << 8)  | data[7];
        r.as_int64_le = static_cast<int64_t>(r.as_uint64_le);
        r.as_int64_be = static_cast<int64_t>(r.as_uint64_be);

        std::memcpy(&r.as_float64_le, &r.as_uint64_le, 8);
        std::memcpy(&r.as_float64_be, &r.as_uint64_be, 8);

        // Windows FILETIME (100ns intervals since Jan 1, 1601)
        // Only valid if > 0 and reasonable year (up to 3000 AD approx)
        if (r.as_uint64_le > 0 && r.as_uint64_le < 200000000000000000ULL) {
            auto secs = (r.as_uint64_le / 10000000ULL) - 11644473600ULL;
            std::time_t t = static_cast<std::time_t>(secs);
#ifdef _WIN32
            std::tm tm_buf;
            if (localtime_s(&tm_buf, &t) == 0) {
                std::ostringstream ts;
                ts << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
                r.as_filetime = ts.str();
            }
#else
            std::tm* tm_buf = std::localtime(&t);
            if (tm_buf) {
                std::ostringstream ts;
                ts << std::put_time(tm_buf, "%Y-%m-%d %H:%M:%S");
                r.as_filetime = ts.str();
            }
#endif
        }
    }
    
    // Unix TS (32-bit LE)
    if (len >= 4 && r.as_int32_le > 0) {
        std::time_t t = r.as_int32_le;
#ifdef _WIN32
        std::tm tm_buf;
        if (localtime_s(&tm_buf, &t) == 0) {
            std::ostringstream ts;
            ts << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
            r.as_unix_timestamp = ts.str();
        }
#else
        std::tm* tm_buf = std::localtime(&t);
        if (tm_buf) {
            std::ostringstream ts;
            ts << std::put_time(tm_buf, "%Y-%m-%d %H:%M:%S");
            r.as_unix_timestamp = ts.str();
        }
#endif
    }

    if (len >= 16) {
        // GUID: {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX}
        char guid_buf[40];
        snprintf(guid_buf, sizeof(guid_buf), "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                 r.as_uint32_le,
                 r.as_uint16_le,
                 (data[5] << 8) | data[4], // Third word is LE as well per MS spec
                 data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
        r.as_guid = guid_buf;
    }

    return r;
}

//─────────────────────────── Hex Buffer (mmap) ─────────────────────────────────

#ifdef _WIN32
struct MmapContext {
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMap = NULL;
    void* mapView = nullptr;
};
#else
struct MmapContext {
    int fd = -1;
    void* mapView = nullptr;
};
#endif

HexBuffer::HexBuffer(const std::filesystem::path& path, bool read_only) 
    : path_(path), read_only_(read_only) 
{
    std::error_code ec;
    file_size_ = std::filesystem::file_size(path, ec);
    if (!ec && file_size_ > 0) {
        ensure_mapped();
    }
}

HexBuffer::~HexBuffer() {
    if (!mmap_handle_) return;
    auto ctx = static_cast<MmapContext*>(mmap_handle_);
#ifdef _WIN32
    if (ctx->mapView) UnmapViewOfFile(ctx->mapView);
    if (ctx->hMap) CloseHandle(ctx->hMap);
    if (ctx->hFile != INVALID_HANDLE_VALUE) CloseHandle(ctx->hFile);
#else
    if (ctx->mapView && ctx->mapView != MAP_FAILED) munmap(ctx->mapView, file_size_);
    if (ctx->fd >= 0) close(ctx->fd);
#endif
    delete ctx;
}

void HexBuffer::ensure_mapped() const {
    if (mmap_handle_ || file_size_ == 0) return;
    
    auto ctx = new MmapContext();
    mmap_handle_ = ctx;

#ifdef _WIN32
    ctx->hFile = CreateFileW(path_.wstring().c_str(), 
                             read_only_ ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE),
                             FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (ctx->hFile == INVALID_HANDLE_VALUE) return;

    ctx->hMap = CreateFileMappingA(ctx->hFile, NULL, 
                                   read_only_ ? PAGE_READONLY : PAGE_READWRITE, 0, 0, NULL);
    if (!ctx->hMap) return;

    ctx->mapView = MapViewOfFile(ctx->hMap, 
                                 read_only_ ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS, 0, 0, 0);
#else
    ctx->fd = open(path_.string().c_str(), read_only_ ? O_RDONLY : O_RDWR);
    if (ctx->fd < 0) return;
    
    ctx->mapView = mmap(nullptr, file_size_, 
                        read_only_ ? PROT_READ : (PROT_READ | PROT_WRITE), 
                        MAP_SHARED, ctx->fd, 0);
#endif
}

int64_t HexBuffer::file_size() const {
    return file_size_; // Does not yet account for inserts/deletes in patches
}

bool HexBuffer::read(int64_t offset, size_t length, std::vector<uint8_t>& out) const {
    if (offset >= file_size_ || offset < 0) return false;
    
    size_t actual_len = std::min(length, static_cast<size_t>(file_size_ - offset));
    out.resize(actual_len);

    if (mmap_handle_) {
        auto ctx = static_cast<MmapContext*>(mmap_handle_);
        if (ctx->mapView) {
            std::memcpy(out.data(), static_cast<uint8_t*>(ctx->mapView) + offset, actual_len);
        }
    }

    apply_patches(offset, out);
    return true;
}

std::vector<uint8_t> HexBuffer::read_page(int64_t page_offset, int page_size) const {
    std::vector<uint8_t> page;
    read(page_offset, page_size, page);
    return page;
}

void HexBuffer::apply_patches(int64_t offset, std::vector<uint8_t>& data) const {
    // Sparse overlay: overwrite mapped bytes with patched bytes
    int64_t end_offset = offset + data.size();
    for (const auto& p : patches_) {
        int64_t p_end = p.offset + p.patched.size();
        
        // Check for overlap
        if (p.offset < end_offset && p_end > offset) {
            int64_t map_start = std::max(offset, p.offset);
            int64_t map_end = std::min(end_offset, p_end);
            
            int64_t data_idx = map_start - offset;
            int64_t patch_idx = map_start - p.offset;
            size_t copy_len = map_end - map_start;
            
            std::memcpy(data.data() + data_idx, p.patched.data() + patch_idx, copy_len);
        }
    }
}

bool HexBuffer::write(int64_t offset, const std::vector<uint8_t>& data) {
    if (read_only_) return false;
    
    std::vector<uint8_t> original;
    read(offset, data.size(), original); // Get current bytes (mapped + existing patches)
    
    HexPatch patch;
    patch.offset = offset;
    patch.original = std::move(original);
    patch.patched = data;
    
    patches_.push_back(std::move(patch));
    redo_stack_.clear();
    return true;
}

bool HexBuffer::undo() {
    if (patches_.empty()) return false;
    redo_stack_.push_back(patches_.back());
    patches_.pop_back();
    return true;
}

bool HexBuffer::redo() {
    if (redo_stack_.empty()) return false;
    patches_.push_back(redo_stack_.back());
    redo_stack_.pop_back();
    return true;
}

DataInterpretation HexBuffer::interpret(int64_t offset) const {
    std::vector<uint8_t> buf;
    read(offset, 16, buf);
    return DataInterpretation::from_bytes(buf.data(), buf.size());
}

bool HexBuffer::insert(int64_t, const std::vector<uint8_t>&) { return false; }
bool HexBuffer::remove(int64_t, size_t) { return false; }
bool HexBuffer::save(const std::filesystem::path&) { return false; }

std::optional<HexSearchResult> HexBuffer::search(const HexSearchOptions&, int64_t) const {
    // Requires streaming chunk search or full mmap search
    return std::nullopt;
}

std::vector<HexSearchResult> HexBuffer::search_all(const HexSearchOptions&) const { return {}; }

} // namespace fo::core
