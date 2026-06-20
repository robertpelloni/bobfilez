#pragma once

#include "fo/core/interfaces.hpp"
#include <memory>
#include <string>

#ifdef FO_HAVE_S3
#include <aws/s3/S3Client.h>
#endif

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
#ifdef FO_HAVE_S3
    std::unique_ptr<Aws::S3::S3Client> s3_client_;
#endif
};

} // namespace fo::core
