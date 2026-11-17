#include "fo/core/provider_registration.hpp"
#include "fo/core/registry.hpp"
#include "fo/core/vault_manager.hpp"

namespace fo::core {

// Explicit registration (called from register_all_providers): the previous
// TU-local static initializer plus a never-called force_register_*() meant
// Registry<IVaultManager> stayed empty in binaries that pull this object
// from fo_core.lib only on demand, so the C API's
// fo_bobfilez_vault_initialize() got nullptr from create("aes256_gcm").
void register_vault_manager() {
    Registry<IVaultManager>::instance().add("aes256_gcm", []() {
        return std::make_unique<VaultManager>();
    });
}

} // namespace fo::core
