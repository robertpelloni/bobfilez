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
            // Note: Since S3 requires bucket credentials, a more robust 
            // factory might parse the URI here. For now, returning an 
            // unconfigured scanner, expecting uri parsing later or via CLI setup.
            return std::make_unique<S3Scanner>("", ""); 
        });
#endif
    }
} registrar;

} // namespace

} // namespace fo::core
