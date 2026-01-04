#include "fo/core/export.hpp"
#include "fo/core/thumbnail.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace fo::core {

std::string Exporter::format_size(std::uintmax_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double size = static_cast<double>(bytes);
    while (size >= 1024.0 && unit < 4) {
        size /= 1024.0;
        ++unit;
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unit];
    return oss.str();
}

std::string Exporter::format_time(std::chrono::file_clock::time_point tp) {
    // Convert file_clock to system_clock (C++20)
    auto sys_tp = std::chrono::clock_cast<std::chrono::system_clock>(tp);
    auto t = std::chrono::system_clock::to_time_t(sys_tp);
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%dT%H:%M:%S");
    return oss.str();
}

ScanStats Exporter::compute_stats(const std::vector<FileInfo>& files,
                                  const std::vector<DuplicateGroup>& duplicates) {
    ScanStats stats;
    for (const auto& f : files) {
        if (f.is_dir) {
            stats.total_directories++;
        } else {
            stats.total_files++;
            stats.total_size += f.size;
        }
    }
    for (const auto& g : duplicates) {
        stats.duplicate_groups++;
        stats.duplicate_files += g.files.size();
        stats.duplicate_size += g.size * (g.files.size() - 1); // Wasted space
    }
    return stats;
}

std::string Exporter::json_escape(const std::string& s) {
    std::ostringstream oss;
    for (char c : s) {
        switch (c) {
            case '"': oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default: oss << c; break;
        }
    }
    return oss.str();
}

std::string Exporter::csv_escape(const std::string& s) {
    if (s.find(',') != std::string::npos || s.find('"') != std::string::npos || s.find('\n') != std::string::npos) {
        std::ostringstream oss;
        oss << '"';
        for (char c : s) {
            if (c == '"') oss << "\"\"";
            else oss << c;
        }
        oss << '"';
        return oss.str();
    }
    return s;
}

std::string Exporter::html_escape(const std::string& s) {
    std::ostringstream oss;
    for (char c : s) {
        switch (c) {
            case '&': oss << "&amp;"; break;
            case '<': oss << "&lt;"; break;
            case '>': oss << "&gt;"; break;
            case '"': oss << "&quot;"; break;
            default: oss << c; break;
        }
    }
    return oss.str();
}

void Exporter::to_json(std::ostream& out, 
                       const std::vector<FileInfo>& files,
                       const std::vector<DuplicateGroup>& duplicates,
                       const ScanStats& stats) {
    out << "{\n";
    out << "  \"stats\": {\n";
    out << "    \"total_files\": " << stats.total_files << ",\n";
    out << "    \"total_directories\": " << stats.total_directories << ",\n";
    out << "    \"total_size\": " << stats.total_size << ",\n";
    out << "    \"total_size_human\": \"" << format_size(stats.total_size) << "\",\n";
    out << "    \"duplicate_groups\": " << stats.duplicate_groups << ",\n";
    out << "    \"duplicate_files\": " << stats.duplicate_files << ",\n";
    out << "    \"duplicate_size\": " << stats.duplicate_size << ",\n";
    out << "    \"duplicate_size_human\": \"" << format_size(stats.duplicate_size) << "\"\n";
    out << "  },\n";
    
    out << "  \"files\": [\n";
    for (size_t i = 0; i < files.size(); ++i) {
        const auto& f = files[i];
        out << "    {\n";
        out << "      \"id\": " << f.id << ",\n";
        out << "      \"path\": \"" << json_escape(f.path.string()) << "\",\n";
        out << "      \"size\": " << f.size << ",\n";
        out << "      \"size_human\": \"" << format_size(f.size) << "\",\n";
        out << "      \"mtime\": \"" << format_time(f.mtime) << "\",\n";
        out << "      \"is_dir\": " << (f.is_dir ? "true" : "false") << "\n";
        out << "    }" << (i + 1 < files.size() ? "," : "") << "\n";
    }
    out << "  ],\n";
    
    out << "  \"duplicates\": [\n";
    for (size_t i = 0; i < duplicates.size(); ++i) {
        const auto& g = duplicates[i];
        out << "    {\n";
        out << "      \"size\": " << g.size << ",\n";
        out << "      \"fast64\": \"" << json_escape(g.fast64) << "\",\n";
        out << "      \"files\": [\n";
        for (size_t j = 0; j < g.files.size(); ++j) {
            out << "        \"" << json_escape(g.files[j].path.string()) << "\"";
            out << (j + 1 < g.files.size() ? "," : "") << "\n";
        }
        out << "      ]\n";
        out << "    }" << (i + 1 < duplicates.size() ? "," : "") << "\n";
    }
    out << "  ]\n";
    out << "}\n";
}

void Exporter::to_csv(std::ostream& out, const std::vector<FileInfo>& files) {
    // CSV Header
    out << "id,path,size,size_human,mtime,is_dir\n";
    for (const auto& f : files) {
        out << f.id << ","
            << csv_escape(f.path.string()) << ","
            << f.size << ","
            << csv_escape(format_size(f.size)) << ","
            << csv_escape(format_time(f.mtime)) << ","
            << (f.is_dir ? "true" : "false") << "\n";
    }
}

void Exporter::duplicates_to_csv(std::ostream& out, const std::vector<DuplicateGroup>& duplicates) {
    // CSV Header
    out << "group_id,size,size_human,fast64,file_path\n";
    int group_id = 1;
    for (const auto& g : duplicates) {
        for (const auto& f : g.files) {
            out << group_id << ","
                << g.size << ","
                << csv_escape(format_size(g.size)) << ","
                << csv_escape(g.fast64) << ","
                << csv_escape(f.path.string()) << "\n";
        }
        ++group_id;
    }
}

void Exporter::to_html(std::ostream& out,
                       const std::vector<FileInfo>& files,
                       const std::vector<DuplicateGroup>& duplicates,
                       const ScanStats& stats,
                       bool include_thumbnails) {
    out << R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>filez Report</title>
<style>
body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; margin: 20px; }
h1, h2 { color: #333; }
.stats { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; margin-bottom: 30px; }
.stat-card { background: #f8f9fa; border-radius: 8px; padding: 15px; text-align: center; }
.stat-value { font-size: 24px; font-weight: bold; color: #007bff; }
.stat-label { color: #666; }
table { width: 100%; border-collapse: collapse; margin-bottom: 30px; }
th, td { border: 1px solid #ddd; padding: 10px; text-align: left; }
th { background: #007bff; color: white; }
tr:nth-child(even) { background: #f8f9fa; }
.dup-group { background: #fff3cd; }
.dup-member { padding-left: 30px; }
.thumbnail { max-width: 100px; max-height: 100px; border-radius: 4px; }
.thumb-cell { width: 110px; text-align: center; }
</style>
</head>
<body>
<h1>filez Report</h1>
)";

    // Stats section
    out << "<div class=\"stats\">\n";
    out << "<div class=\"stat-card\"><div class=\"stat-value\">" << stats.total_files << "</div><div class=\"stat-label\">Files</div></div>\n";
    out << "<div class=\"stat-card\"><div class=\"stat-value\">" << html_escape(format_size(stats.total_size)) << "</div><div class=\"stat-label\">Total Size</div></div>\n";
    out << "<div class=\"stat-card\"><div class=\"stat-value\">" << stats.duplicate_groups << "</div><div class=\"stat-label\">Duplicate Groups</div></div>\n";
    out << "<div class=\"stat-card\"><div class=\"stat-value\">" << html_escape(format_size(stats.duplicate_size)) << "</div><div class=\"stat-label\">Wasted Space</div></div>\n";
    out << "</div>\n";

    // Duplicates section
    if (!duplicates.empty()) {
        if (include_thumbnails) {
            out << "<h2>Duplicate Files</h2>\n<table>\n<tr><th class=\"thumb-cell\">Preview</th><th>Group</th><th>Size</th><th>Hash</th><th>Files</th></tr>\n";
        } else {
            out << "<h2>Duplicate Files</h2>\n<table>\n<tr><th>Group</th><th>Size</th><th>Hash</th><th>Files</th></tr>\n";
        }
        int group_id = 1;
        for (const auto& g : duplicates) {
            // Generate thumbnail for first file in group if it's an image
            std::string thumb_html;
            if (include_thumbnails && !g.files.empty() && ThumbnailGenerator::is_image_file(g.files[0].path)) {
                auto thumb = ThumbnailGenerator::generate_base64(g.files[0].path);
                if (thumb) {
                    thumb_html = "<img class=\"thumbnail\" src=\"data:image/jpeg;base64," + *thumb + "\" alt=\"thumbnail\">";
                }
            }

            if (include_thumbnails) {
                out << "<tr class=\"dup-group\"><td class=\"thumb-cell\" rowspan=\"" << g.files.size() << "\">" << thumb_html
                    << "</td><td rowspan=\"" << g.files.size() << "\">" << group_id
                    << "</td><td rowspan=\"" << g.files.size() << "\">" << html_escape(format_size(g.size))
                    << "</td><td rowspan=\"" << g.files.size() << "\">" << html_escape(g.fast64.substr(0, 16)) << "..."
                    << "</td><td>" << html_escape(g.files[0].path.string()) << "</td></tr>\n";
            } else {
                out << "<tr class=\"dup-group\"><td rowspan=\"" << g.files.size() << "\">" << group_id
                    << "</td><td rowspan=\"" << g.files.size() << "\">" << html_escape(format_size(g.size))
                    << "</td><td rowspan=\"" << g.files.size() << "\">" << html_escape(g.fast64.substr(0, 16)) << "..."
                    << "</td><td>" << html_escape(g.files[0].path.string()) << "</td></tr>\n";
            }
            for (size_t i = 1; i < g.files.size(); ++i) {
                out << "<tr><td>" << html_escape(g.files[i].path.string()) << "</td></tr>\n";
            }
            ++group_id;
        }
        out << "</table>\n";
    }

    // Files section (limited to first 100 for performance)
    out << "<h2>Files (" << files.size() << " total";
    if (files.size() > 100) out << ", showing first 100";
    if (include_thumbnails) {
        out << ")</h2>\n<table>\n<tr><th class=\"thumb-cell\">Preview</th><th>Path</th><th>Size</th><th>Modified</th></tr>\n";
    } else {
        out << ")</h2>\n<table>\n<tr><th>Path</th><th>Size</th><th>Modified</th></tr>\n";
    }
    size_t limit = std::min(files.size(), size_t(100));
    for (size_t i = 0; i < limit; ++i) {
        const auto& f = files[i];
        if (!f.is_dir) {
            std::string thumb_html;
            if (include_thumbnails && ThumbnailGenerator::is_image_file(f.path)) {
                auto thumb = ThumbnailGenerator::generate_base64(f.path);
                if (thumb) {
                    thumb_html = "<img class=\"thumbnail\" src=\"data:image/jpeg;base64," + *thumb + "\" alt=\"thumbnail\">";
                }
            }

            if (include_thumbnails) {
                out << "<tr><td class=\"thumb-cell\">" << thumb_html << "</td><td>" << html_escape(f.path.string()) << "</td><td>"
                    << html_escape(format_size(f.size)) << "</td><td>"
                    << html_escape(format_time(f.mtime)) << "</td></tr>\n";
            } else {
                out << "<tr><td>" << html_escape(f.path.string()) << "</td><td>"
                    << html_escape(format_size(f.size)) << "</td><td>"
                    << html_escape(format_time(f.mtime)) << "</td></tr>\n";
            }
        }
    }
    out << "</table>\n";
    out << "</body>\n</html>\n";
}

bool Exporter::export_to_file(const std::filesystem::path& output_path,
                              const std::vector<FileInfo>& files,
                              const std::vector<DuplicateGroup>& duplicates,
                              const ScanStats& stats,
                              ExportFormat format,
                              bool include_thumbnails) {
    std::ofstream out(output_path);
    if (!out) return false;

    switch (format) {
        case ExportFormat::JSON:
            to_json(out, files, duplicates, stats);
            break;
        case ExportFormat::CSV:
            to_csv(out, files);
            break;
        case ExportFormat::HTML:
            to_html(out, files, duplicates, stats, include_thumbnails);
            break;
    }
    return out.good();
}

} // namespace fo::core

