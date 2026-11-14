import re

with open("core/include/fo/c_api/bobfilez_c_api.h", "r") as f:
    header = f.read()

vault_header = """
// Vault Operations
FO_EXPORT int fo_bobfilez_vault_initialize(const char* vault_path, const char* password);
FO_EXPORT int fo_bobfilez_vault_lock(const char* file_path);
FO_EXPORT int fo_bobfilez_vault_unlock(const char* vault_id, const char* dest_path);
FO_EXPORT char* fo_bobfilez_vault_list();
"""
header = header.replace('// Tag Operations', vault_header + '\n// Tag Operations')
with open("core/include/fo/c_api/bobfilez_c_api.h", "w") as f:
    f.write(header)

with open("core/c_api/bobfilez_c_api.cpp", "r") as f:
    cpp = f.read()

vault_cpp = """
#include "fo/core/vault_manager.hpp"
#include <nlohmann/json.hpp>

static fo::core::VaultManager g_vault_manager;
static bool g_vault_initialized = false;

extern "C" int fo_bobfilez_vault_initialize(const char* vault_path, const char* password) {
    if (!vault_path || !password) return 0;
    try {
        g_vault_initialized = g_vault_manager.initialize(vault_path, password);
        return g_vault_initialized ? 1 : 0;
    } catch (...) {
        return 0;
    }
}

extern "C" int fo_bobfilez_vault_lock(const char* file_path) {
    if (!g_vault_initialized || !file_path) return 0;
    try {
        return g_vault_manager.lock_file(file_path) ? 1 : 0;
    } catch (...) {
        return 0;
    }
}

extern "C" int fo_bobfilez_vault_unlock(const char* vault_id, const char* dest_path) {
    if (!g_vault_initialized || !vault_id || !dest_path) return 0;
    try {
        return g_vault_manager.unlock_file(vault_id, dest_path) ? 1 : 0;
    } catch (...) {
        return 0;
    }
}

extern "C" char* fo_bobfilez_vault_list() {
    if (!g_vault_initialized) return nullptr;
    try {
        auto entries = g_vault_manager.list_contents();
        nlohmann::json j = nlohmann::json::array();
        for (const auto& e : entries) {
            j.push_back({
                {"id", e.id},
                {"original_name", e.original_name},
                {"size", e.size},
                {"added_at", e.added_at}
            });
        }
        std::string s = j.dump();
        char* cstr = (char*)malloc(s.length() + 1);
        if (cstr) {
            strcpy(cstr, s.c_str());
        }
        return cstr;
    } catch (...) {
        return nullptr;
    }
}
"""

# append to bottom
cpp = cpp + '\n' + vault_cpp

with open("core/c_api/bobfilez_c_api.cpp", "w") as f:
    f.write(cpp)
