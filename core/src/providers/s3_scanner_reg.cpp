#include "fo/core/engine.hpp"
#include "fo/core/interfaces.hpp"

#ifdef FO_HAVE_S3
#include "fo/core/providers/s3_scanner.hpp"
#include <aws/core/Aws.h>
#endif

namespace fo::core {

void register_scanner_s3() {
#ifdef FO_HAVE_S3
    Registry<IFileScanner>::instance().add("s3", []() {
        const char* b = std::getenv("S3_BUCKET");
        const char* p = std::getenv("S3_PREFIX");
        std::string bucket = b ? b : "";
        std::string prefix = p ? p : "";
        return std::make_unique<S3Scanner>(bucket, prefix); 
    });
#endif
}

#ifdef FO_HAVE_S3
static Aws::SDKOptions aws_options;
void init_aws_api() {
    aws_options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Off;
    Aws::InitAPI(aws_options);
}
void shutdown_aws_api() {
    Aws::ShutdownAPI(aws_options);
}
#endif

} // namespace fo::core
