#include "fo/core/engine.hpp"
#include "fo/core/interfaces.hpp"

#ifdef FO_HAVE_CPR
#include "fo/core/providers/gdrive_scanner.hpp"
#endif

namespace fo::core {

void register_scanner_gdrive() {
#ifdef FO_HAVE_CPR
    Registry<IFileScanner>::instance().add("gdrive", []() {
        // Note: Google Drive requires an OAuth access token.
        // For now, returning an unconfigured scanner, expecting uri parsing later or via CLI environment variables.
        const char* token = std::getenv("GDRIVE_TOKEN");
        std::string t = token ? token : "";
        return std::make_unique<GDriveScanner>(t); 
    });
#endif
}

} // namespace fo::core
