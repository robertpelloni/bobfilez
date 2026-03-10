#include "fo/core/provider_registration.hpp"
#include <iostream>

namespace fo::core {
    // Forward declarations for individual provider registration functions.
    void register_hasher_dhash();
    void register_linter_std();
    void register_scanner_s3();
    void register_scanner_gdrive();
    void register_scanner_azure();

    void register_extended_providers() {
        register_hasher_dhash();
        register_scanner_s3();
        register_scanner_gdrive();
        register_scanner_azure();
        // Add calls to other provider registration functions here.
    }

    void register_all_providers() {
        register_scanner_std();
#ifdef _WIN32
        register_scanner_win32();
#endif
        register_scanner_dirent();
        register_hasher_fast64();
        register_hasher_sha256();
        register_hasher_xxhash();
        register_hasher_blake3();
        register_metadata_tinyexif();
        register_linter_std(); // Added
        
        register_extended_providers();
    }
}
