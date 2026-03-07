#pragma once

#include "fo/core/interfaces.hpp"
#include <string>
#include <vector>

namespace fo::core {

class GDriveScanner : public IFileScanner {
public:
    GDriveScanner(const std::string& access_token);
    ~GDriveScanner() override;

    std::string name() const override { return "gdrive"; }

    std::vector<FileInfo> scan(
        const std::vector<std::filesystem::path>& roots,
        const std::vector<std::string>& include_exts,
        bool follow_symlinks) override;

private:
    std::string access_token_;
};

} // namespace fo::core
