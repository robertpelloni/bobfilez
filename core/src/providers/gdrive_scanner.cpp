#include "fo/core/providers/gdrive_scanner.hpp"
#include <iostream>
#include <chrono>

#ifdef FO_HAVE_CPR
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#endif

namespace fo::core {

GDriveScanner::GDriveScanner(const std::string& access_token)
    : access_token_(access_token)
{
}

GDriveScanner::~GDriveScanner() = default;

std::vector<FileInfo> GDriveScanner::scan(
    const std::vector<std::filesystem::path>& roots,
    const std::vector<std::string>& include_exts,
    bool follow_symlinks) {
    
    (void)roots;
    (void)include_exts;
    (void)follow_symlinks;
    
    std::vector<FileInfo> result;
#ifdef FO_HAVE_CPR
    if (access_token_.empty()) {
        std::cerr << "GDriveScanner: access token is empty!" << std::endl;
        return result;
    }

    // Google Drive REST API endpoint for listing files
    std::string url = "https://www.googleapis.com/drive/v3/files?fields=files(id,name,size,modifiedTime,mimeType)";
    
    auto response = cpr::Get(cpr::Url{url},
                             cpr::Header{{"Authorization", "Bearer " + access_token_}});

    if (response.status_code == 200) {
        try {
            auto json_resp = nlohmann::json::parse(response.text);
            if (json_resp.contains("files") && json_resp["files"].is_array()) {
                for (const auto& file : json_resp["files"]) {
                    if (file["mimeType"] == "application/vnd.google-apps.folder") {
                        continue; // Skip directories
                    }
                    
                    FileInfo fi;
                    // We use gdrive:// followed by the file ID or Name 
                    std::string file_id = file.value("id", "");
                    std::string file_name = file.value("name", "unknown");
                    fi.uri = "gdrive://" + file_name + "::" + file_id;
                    fi.is_dir = false;
                    
                    if (file.contains("size")) {
                        // size is returned as a string in JSON
                        fi.size = std::stoull(file.value("size", "0"));
                    }

                    if (file.contains("modifiedTime")) {
                        // Example format: "2012-06-04T12:00:00.000Z"
                        std::string time_str = file["modifiedTime"];
                        std::tm tm = {};
                        int ms;
                        // Basic parsing, C++20 standard chrono parsing or simple sscanf
                        if (sscanf(time_str.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d", 
                                   &tm.tm_year, &tm.tm_mon, &tm.tm_mday,
                                   &tm.tm_hour, &tm.tm_min, &tm.tm_sec) == 6) {
                            tm.tm_year -= 1900;
                            tm.tm_mon -= 1;
                            std::time_t t = std::strftime(nullptr, 0, nullptr, nullptr) ? 0 : 0; // dummy
                            // Note: gmtime/mktime platform specifics apply here.
                            fi.mtime = std::chrono::clock_cast<std::chrono::file_clock>(
                                        std::chrono::system_clock::from_time_t(0)); // Placeholder date parsing
                        }
                    }

                    result.push_back(fi);
                }
            }
        } catch (const nlohmann::json::exception& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    } else {
        std::cerr << "GDrive API error: " << response.status_code << " " << response.text << std::endl;
    }
#else
    std::cerr << "GDriveScanner requires CPR dependency built into the engine." << std::endl;
#endif
    return result;
}

} // namespace fo::core
