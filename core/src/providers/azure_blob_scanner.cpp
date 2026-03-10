#include "fo/core/providers/azure_blob_scanner.hpp"
#include <iostream>
#include <chrono>

#ifdef FO_HAVE_AZURE_STORAGE
#include <azure/storage/blobs.hpp>
#endif

namespace fo::core {

AzureBlobScanner::AzureBlobScanner(const std::string& connection_str, const std::string& container)
    : connection_str_(connection_str), container_(container)
{
}

AzureBlobScanner::~AzureBlobScanner() = default;

std::vector<FileInfo> AzureBlobScanner::scan(
    const std::vector<std::filesystem::path>& roots,
    const std::vector<std::string>& include_exts,
    bool follow_symlinks) {

    (void)include_exts;
    (void)follow_symlinks;

    // Allow overriding container via roots (e.g. "azure://mycontainer/prefix")
    std::string prefix;
    if (!roots.empty()) {
        std::string root_str = roots[0].string();
        if (root_str.find("azure://") == 0) {
            auto path_part = root_str.substr(8); // skip "azure://"
            auto slash = path_part.find('/');
            if (slash != std::string::npos) {
                container_ = path_part.substr(0, slash);
                prefix = path_part.substr(slash + 1);
            } else {
                container_ = path_part;
            }
        }
    }

    std::vector<FileInfo> result;

#ifdef FO_HAVE_AZURE_STORAGE
    try {
        auto client = Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
            connection_str_, container_);

        Azure::Storage::Blobs::ListBlobsOptions options;
        if (!prefix.empty()) {
            options.Prefix = prefix;
        }

        // Paginated listing via continuation token
        auto pager = client.ListBlobs(options);
        for (;;) {
            for (const auto& blob : pager.Blobs) {
                if (blob.BlobSize == 0 && !blob.Name.empty() && blob.Name.back() == '/') {
                    continue; // Skip directory markers
                }

                FileInfo fi;
                fi.uri = "azure://" + container_ + "/" + blob.Name;
                fi.size = blob.BlobSize;
                fi.is_dir = false;

                // Convert Azure DateTime to file_clock
                // Convert Azure DateTime to file_clock. LastModified is consistently present in Blob Item Details.
                {
                    auto azure_tp = blob.Details.LastModified;
                    // Fallback to std::chrono ISO string parsing to bypass MSVC integration bugs entirely.
                    std::string time_str = azure_tp.ToString(Azure::DateTime::DateFormat::Rfc3339);
                    std::chrono::system_clock::time_point sys_tp;
                    std::istringstream in{time_str};
                    in >> std::chrono::parse("%Y-%m-%dT%H:%M:%SZ", sys_tp);
                    fi.mtime = std::chrono::clock_cast<std::chrono::file_clock>(sys_tp);
                }

                result.push_back(fi);
            }

            if (!pager.NextPageToken.HasValue()) {
                break;
            }
            options.ContinuationToken = pager.NextPageToken;
            pager = client.ListBlobs(options);
        }
    } catch (const Azure::Core::RequestFailedException& e) {
        std::cerr << "Azure Blob Storage error: " << e.what() << std::endl;
    }
#else
    (void)prefix;
    std::cerr << "AzureBlobScanner requires azure-storage-blobs-cpp dependency." << std::endl;
#endif
    return result;
}

} // namespace fo::core
