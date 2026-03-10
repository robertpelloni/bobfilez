#include "fo/core/providers/s3_scanner.hpp"
#include <iostream>
#include <chrono>

#ifdef FO_HAVE_S3
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/auth/signer/AWSAuthV4Signer.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/ListObjectsV2Request.h>
#include <aws/s3/model/HeadObjectRequest.h>
#endif

namespace fo::core {

S3Scanner::S3Scanner(const std::string& bucket, const std::string& prefix)
    : bucket_(bucket), prefix_(prefix)
{
#ifdef FO_HAVE_S3
    Aws::Client::ClientConfiguration config;
    bool use_virtual = true;
    std::shared_ptr<Aws::Auth::AWSCredentialsProvider> cred_provider = nullptr;
    
    if (const char* env_ep = std::getenv("S3_ENDPOINT_URL")) {
        config.endpointOverride = Aws::String(env_ep);
        config.scheme = Aws::Http::Scheme::HTTP; // It's localhost, force HTTP
        use_virtual = false; // Force path-style for localhost
        
        // Mock credentials to avoid AWS SDK hitting EC2 metadata service
        cred_provider = std::make_shared<Aws::Auth::SimpleAWSCredentialsProvider>("mock_key", "mock_secret");
    }
    
    if (cred_provider) {
        s3_client_ = std::make_unique<Aws::S3::S3Client>(
            cred_provider, config,
            Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::RequestDependent,
            use_virtual
        );
    } else {
        s3_client_ = std::make_unique<Aws::S3::S3Client>(
            config,
            Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::RequestDependent,
            use_virtual
        );
    }
#endif
}

S3Scanner::~S3Scanner() = default;

std::vector<FileInfo> S3Scanner::scan(
    const std::vector<std::filesystem::path>& roots,
    const std::vector<std::string>& include_exts,
    bool follow_symlinks) {
    (void)include_exts;
    (void)follow_symlinks;
    std::string custom_prefix = prefix_;
    if (!roots.empty()) {
        std::string root_str = roots[0].string();
        // naive s3:// check
        if (root_str.find("s3://") == 0) {
            auto path_part = root_str.substr(5);
            auto slash = path_part.find('/');
            if (slash != std::string::npos) {
                // we have a bucket and maybe prefix
                bucket_ = path_part.substr(0, slash);
                custom_prefix = path_part.substr(slash + 1);
            } else {
                bucket_ = path_part;
                custom_prefix = "";
            }
        }
    }
    std::vector<FileInfo> result;
#ifdef FO_HAVE_S3
    Aws::S3::Model::ListObjectsV2Request request;
    request.WithBucket(bucket_);
    if (!custom_prefix.empty()) {
        request.WithPrefix(custom_prefix);
    }

    bool isDone = false;
    while (!isDone) {
        auto outcome = s3_client_->ListObjectsV2(request);
        if (outcome.IsSuccess()) {
            const auto& result_chunk = outcome.GetResult().GetContents();
            for (const auto& object : result_chunk) {
                if (object.GetSize() == 0 && object.GetKey().back() == '/') {
                    continue; // Skip directory markers
                }

                FileInfo fi;
                fi.uri = "s3://" + bucket_ + "/" + object.GetKey();
                fi.size = object.GetSize();
                fi.is_dir = false; // S3 objects are files
                
                // Convert AWS Date to time_point
                auto aws_time = object.GetLastModified().Millis();
                if (aws_time > 0) {
                    auto ch_time = std::chrono::system_clock::time_point(std::chrono::milliseconds(aws_time));
                    // Convert to file_clock. This can be tricky, typically file_clock matches system_clock on modern MSVC, 
                    // but we do a cast for safety.
                    fi.mtime = std::chrono::clock_cast<std::chrono::file_clock>(ch_time);
                }
                
                result.push_back(fi);
            }

            isDone = !outcome.GetResult().GetIsTruncated();
            if (!isDone) {
                request.SetContinuationToken(outcome.GetResult().GetNextContinuationToken());
            }
        } else {
            std::cerr << "S3 ListObjectsV2 failed: " << outcome.GetError().GetMessage() << std::endl;
            break;
        }
    }
#endif
    return result;
}

} // namespace fo::core
