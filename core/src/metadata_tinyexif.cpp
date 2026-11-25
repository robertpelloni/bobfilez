#include "fo/core/interfaces.hpp"
#include "fo/core/registry.hpp"
#include "../../libs/TinyEXIF/TinyEXIF.h"
#include <fstream>
#include <cmath>
#include <cstring>
#include <ctime>

namespace fo::core {

class TinyExifMetadataProvider : public IMetadataProvider {
public:
    std::string name() const override { return "tinyexif"; }
    bool read(const std::filesystem::path& p, ImageMetadata& out) override;
};

// Parse EXIF datetime string (YYYY:MM:DD HH:MM:SS) to system_clock time_point
static bool parse_exif_datetime(const std::string& s, std::chrono::system_clock::time_point& out) {
    if (s.size() < 19) return false;
    std::tm tm{};
    if (std::sscanf(s.c_str(), "%d:%d:%d %d:%d:%d",
                    &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                    &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 6) return false;
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    std::time_t tt = std::mktime(&tm);
    if (tt == static_cast<std::time_t>(-1)) return false;
    out = std::chrono::system_clock::from_time_t(tt);
    return true;
}

bool TinyExifMetadataProvider::read(const std::filesystem::path& p, fo::core::ImageMetadata& out) {
    std::ifstream f(p, std::ios::binary);
    if (!f) return false;

    TinyEXIF::EXIFInfo exif(f);
    if (exif.Fields == 0) return false; // no EXIF data

    // Priority: DateTimeOriginal > DateTimeDigitized > DateTime
    if (!exif.DateTimeOriginal.empty()) {
        if (parse_exif_datetime(exif.DateTimeOriginal, out.date.taken)) {
            out.date.has_taken = true;
            out.date.source_string = exif.DateTimeOriginal;
            out.date.source_field = "EXIF:DateTimeOriginal";
        }
    } else if (!exif.DateTimeDigitized.empty()) {
        if (parse_exif_datetime(exif.DateTimeDigitized, out.date.taken)) {
            out.date.has_taken = true;
            out.date.source_string = exif.DateTimeDigitized;
            out.date.source_field = "EXIF:DateTimeDigitized";
        }
    } else if (!exif.DateTime.empty()) {
        if (parse_exif_datetime(exif.DateTime, out.date.taken)) {
            out.date.has_taken = true;
            out.date.source_string = exif.DateTime;
            out.date.source_field = "EXIF:DateTime";
        }
    }

    // GPS. TinyEXIF reports absent GPS with sentinel doubles
    // (DBL_MAX), which the old `!= 0.0` check surfaced as real
    // coordinates ("GPS: 1.79769e+308, ..."). Only finite,
    // in-range values count as GPS.
    {
        const double lat = exif.GeoLocation.Latitude;
        const double lon = exif.GeoLocation.Longitude;
        if (std::isfinite(lat) && std::isfinite(lon) &&
            lat >= -90.0 && lat <= 90.0 && lon >= -180.0 && lon <= 180.0 &&
            (lat != 0.0 || lon != 0.0)) {
            out.has_gps = true;
            out.gps_lat = lat;
            out.gps_lon = lon;
        }
    }

    return out.date.has_taken || out.has_gps;
}

// Static registration
static bool reg_metadata_tinyexif = [](){
    Registry<IMetadataProvider>::instance().add("tinyexif", [](){ return std::make_unique<TinyExifMetadataProvider>(); });
    return true;
}();

void register_metadata_tinyexif() { (void)reg_metadata_tinyexif; }

} // namespace fo::core
