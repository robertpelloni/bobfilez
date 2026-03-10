#include "fo/core/engine.hpp"
#include "fo/core/interfaces.hpp"

#ifdef FO_HAVE_AZURE_STORAGE
#include "fo/core/providers/azure_blob_scanner.hpp"
#endif

namespace fo::core {

void register_scanner_azure() {
#ifdef FO_HAVE_AZURE_STORAGE
    Registry<IFileScanner>::instance().add("azure", []() {
        const char* conn = std::getenv("AZURE_STORAGE_CONNECTION_STRING");
        const char* cont = std::getenv("AZURE_CONTAINER");
        std::string connection_str = conn ? conn : "";
        std::string container = cont ? cont : "";
        return std::make_unique<AzureBlobScanner>(connection_str, container);
    });
#endif
}

} // namespace fo::core
