/// @file cloud_provider.cpp
/// @brief Implementation of the Cloud Provider management system.

#include "fo/core/cloud_provider_interface.hpp"
#include "fo/core/registry.hpp"
#include <iostream>

namespace fo::core {

void CloudProviderManager::add_account(const CloudAccount& account) {
    accounts_[account.id] = account;
    std::cout << "[Cloud] Added account: " << account.display_name << " (" << account.provider << ")\n";
}

void CloudProviderManager::remove_account(const std::string& id) {
    accounts_.erase(id);
}

std::vector<CloudAccount> CloudProviderManager::get_accounts() {
    std::vector<CloudAccount> res;
    for (auto& [id, acc] : accounts_) res.push_back(acc);
    return res;
}

std::shared_ptr<ICloudProvider> CloudProviderManager::get_provider(const std::string& account_id) {
    // In a full implementation, this returns the specialized S3/GDrive provider
    // that uses the existing scanner/hasher backend for actual I/O.
    return nullptr; 
}

} // namespace fo::core
