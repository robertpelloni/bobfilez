#include "fo/core/engine.hpp"
#include "fo/core/interfaces.hpp"

#ifdef FO_HAVE_S3
#include "fo/core/providers/s3_scanner.hpp"
#endif

namespace fo::core {

namespace {

// Anonymous namespace for registry registration
struct S3ScannerRegistrar {
    S3ScannerRegistrar() {
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
} registrar;

} // namespace

} // namespace fo::core
