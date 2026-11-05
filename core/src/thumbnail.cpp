#include "fo/core/thumbnail.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>

#ifdef _WIN32
// Prevent min/max macros from conflicting with std::min/std::max
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Include std::algorithm first for min/max
#include <algorithm>

namespace Gdiplus {
    using std::min;
    using std::max;
}

#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#endif

namespace fo::core {

bool ThumbnailGenerator::is_image_file(const std::filesystem::path& path) {
    static const std::vector<std::string> image_exts = {
        ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".tiff", ".tif", ".webp"
    };
    
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    
    return std::find(image_exts.begin(), image_exts.end(), ext) != image_exts.end();
}

std::string ThumbnailGenerator::base64_encode(const std::vector<unsigned char>& data) {
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    result.reserve((data.size() + 2) / 3 * 4);
    
    for (size_t i = 0; i < data.size(); i += 3) {
        unsigned int n = static_cast<unsigned int>(data[i]) << 16;
        if (i + 1 < data.size()) n |= static_cast<unsigned int>(data[i + 1]) << 8;
        if (i + 2 < data.size()) n |= static_cast<unsigned int>(data[i + 2]);
        
        result += chars[(n >> 18) & 0x3F];
        result += chars[(n >> 12) & 0x3F];
        result += (i + 1 < data.size()) ? chars[(n >> 6) & 0x3F] : '=';
        result += (i + 2 < data.size()) ? chars[n & 0x3F] : '=';
    }
    
    return result;
}

#ifdef _WIN32

// GDI+ initialization helper
class GdiplusInit {
public:
    GdiplusInit() {
        Gdiplus::GdiplusStartupInput input;
        Gdiplus::GdiplusStartup(&token_, &input, nullptr);
    }
    ~GdiplusInit() {
        Gdiplus::GdiplusShutdown(token_);
    }
private:
    ULONG_PTR token_ = 0;
};

// Get CLSID for image encoder
static bool GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0, size = 0;
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) return false;
    
    std::vector<unsigned char> buffer(size);
    auto* encoders = reinterpret_cast<Gdiplus::ImageCodecInfo*>(buffer.data());
    Gdiplus::GetImageEncoders(num, size, encoders);
    
    for (UINT i = 0; i < num; ++i) {
        if (wcscmp(encoders[i].MimeType, format) == 0) {
            *pClsid = encoders[i].Clsid;
            return true;
        }
    }
    return false;
}

std::optional<std::string> ThumbnailGenerator::generate_base64(
    const std::filesystem::path& path, int max_width, int max_height) {
    
    if (!is_image_file(path)) return std::nullopt;
    if (!std::filesystem::exists(path)) return std::nullopt;
    
    static GdiplusInit gdiplus_init;
    
    // Load image
    std::wstring wpath = path.wstring();
    Gdiplus::Bitmap* original = Gdiplus::Bitmap::FromFile(wpath.c_str());
    if (!original || original->GetLastStatus() != Gdiplus::Ok) {
        delete original;
        return std::nullopt;
    }
    
    // Calculate thumbnail dimensions maintaining aspect ratio
    int orig_w = original->GetWidth();
    int orig_h = original->GetHeight();
    
    if (orig_w == 0 || orig_h == 0) {
        delete original;
        return std::nullopt;
    }
    
    double scale = std::min(
        static_cast<double>(max_width) / orig_w,
        static_cast<double>(max_height) / orig_h
    );
    
    int thumb_w = static_cast<int>(orig_w * scale);
    int thumb_h = static_cast<int>(orig_h * scale);
    
    if (thumb_w < 1) thumb_w = 1;
    if (thumb_h < 1) thumb_h = 1;
    
    // Create thumbnail
    Gdiplus::Bitmap thumbnail(thumb_w, thumb_h, PixelFormat24bppRGB);
    Gdiplus::Graphics graphics(&thumbnail);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    graphics.DrawImage(original, 0, 0, thumb_w, thumb_h);
    delete original;
    
    // Save to memory stream as JPEG
    CLSID jpegClsid;
    if (!GetEncoderClsid(L"image/jpeg", &jpegClsid)) {
        return std::nullopt;
    }
    
    IStream* stream = nullptr;
    if (CreateStreamOnHGlobal(nullptr, TRUE, &stream) != S_OK) {
        return std::nullopt;
    }
    
    Gdiplus::EncoderParameters params;
    ULONG quality = 75;
    params.Count = 1;
    params.Parameter[0].Guid = Gdiplus::EncoderQuality;
    params.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
    params.Parameter[0].NumberOfValues = 1;
    params.Parameter[0].Value = &quality;
    
    if (thumbnail.Save(stream, &jpegClsid, &params) != Gdiplus::Ok) {
        stream->Release();
        return std::nullopt;
    }
    
    // Get stream size and data
    STATSTG stat;
    stream->Stat(&stat, STATFLAG_NONAME);
    ULONG size = static_cast<ULONG>(stat.cbSize.QuadPart);
    
    LARGE_INTEGER zero = {};
    stream->Seek(zero, STREAM_SEEK_SET, nullptr);
    
    std::vector<unsigned char> data(size);
    ULONG read = 0;
    stream->Read(data.data(), size, &read);
    stream->Release();
    
    if (read != size) return std::nullopt;
    
    return base64_encode(data);
}

bool ThumbnailGenerator::generate_to_file(
    const std::filesystem::path& source,
    const std::filesystem::path& dest,
    int max_width, int max_height) {
    
    auto base64 = generate_base64(source, max_width, max_height);
    if (!base64) return false;
    
    // Decode base64 and write to file (simplified - just write raw JPEG)
    // For now, regenerate directly to file
    
    if (!is_image_file(source)) return false;
    if (!std::filesystem::exists(source)) return false;
    
    static GdiplusInit gdiplus_init;
    
    std::wstring wsource = source.wstring();
    Gdiplus::Bitmap* original = Gdiplus::Bitmap::FromFile(wsource.c_str());
    if (!original || original->GetLastStatus() != Gdiplus::Ok) {
        delete original;
        return false;
    }
    
    int orig_w = original->GetWidth();
    int orig_h = original->GetHeight();
    
    double scale = std::min(
        static_cast<double>(max_width) / orig_w,
        static_cast<double>(max_height) / orig_h
    );
    
    int thumb_w = std::max(1, static_cast<int>(orig_w * scale));
    int thumb_h = std::max(1, static_cast<int>(orig_h * scale));
    
    Gdiplus::Bitmap thumbnail(thumb_w, thumb_h, PixelFormat24bppRGB);
    Gdiplus::Graphics graphics(&thumbnail);
    graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    graphics.DrawImage(original, 0, 0, thumb_w, thumb_h);
    delete original;
    
    CLSID jpegClsid;
    if (!GetEncoderClsid(L"image/jpeg", &jpegClsid)) return false;
    
    std::wstring wdest = dest.wstring();
    return thumbnail.Save(wdest.c_str(), &jpegClsid, nullptr) == Gdiplus::Ok;
}

#else
// Non-Windows stub implementations

std::optional<std::string> ThumbnailGenerator::generate_base64(
    const std::filesystem::path& path, int, int) {
    (void)path;
    return std::nullopt; // Not implemented on non-Windows
}

bool ThumbnailGenerator::generate_to_file(
    const std::filesystem::path&, const std::filesystem::path&, int, int) {
    return false; // Not implemented on non-Windows
}

#endif

} // namespace fo::core

