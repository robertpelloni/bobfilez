#include "fo/core/registry.hpp"
#include "fo/core/vault_manager.hpp"

namespace fo::core {
namespace providers {

static bool registered = [](){
    Registry<IVaultManager>::instance().add("aes256_gcm", []() {
        return std::make_unique<VaultManager>();
    });
    return true;
}();

void force_register_vault_manager() {
    (void)registered;
}

} // namespace providers
} // namespace fo::core
