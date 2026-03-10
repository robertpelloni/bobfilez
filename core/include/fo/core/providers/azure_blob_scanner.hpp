#pragma once

#include "fo/core/interfaces.hpp"
#include <string>
#include <memory>

namespace fo::core {

/// Azure Blob Storage scanner — lists blobs in a container via the Azure SDK.
class AzureBlobScanner : public IFileScanner {
public:
    /// @param connection_str  Azure Storage connection string
    /// @param container       Container name
    AzureBlobScanner(const std::string& connection_str, const std::string& container);
    ~AzureBlobScanner() override;

    std::string name() const override { return "azure"; }

    std::vector<FileInfo> scan(
        const std::vector<std::filesystem::path>& roots,
        const std::vector<std::string>& include_exts,
        bool follow_symlinks) override;

private:
    std::string connection_str_;
    std::string container_;
};

} // namespace fo::core
