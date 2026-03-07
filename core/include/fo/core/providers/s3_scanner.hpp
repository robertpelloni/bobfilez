#pragma once

#include "fo/core/interfaces.hpp"
#include <memory>
#include <string>

namespace Aws::S3 {
    class S3Client;
}

namespace fo::core {

class S3Scanner : public IFileScanner {
public:
    S3Scanner(const std::string& bucket, const std::string& prefix = "");
    ~S3Scanner() override;

    std::string name() const override { return "s3"; }

    std::vector<FileInfo> scan(
        const std::vector<std::filesystem::path>& roots,
        const std::vector<std::string>& include_exts,
        bool follow_symlinks) override;

private:
    std::string bucket_;
    std::string prefix_;
    std::unique_ptr<Aws::S3::S3Client> s3_client_;
};

} // namespace fo::core
