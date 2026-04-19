with open("core/c_api/bobfilez_c_api.cpp", "r") as f:
    text = f.read()

text = text.replace('static fo::core::VaultManager g_vault_manager;', 'static std::unique_ptr<fo::core::IVaultManager> g_vault_manager;')

text = text.replace('g_vault_manager.initialize', 'g_vault_manager->initialize')
text = text.replace('g_vault_manager.lock_file', 'g_vault_manager->lock_file')
text = text.replace('g_vault_manager.unlock_file', 'g_vault_manager->unlock_file')
text = text.replace('g_vault_manager.list_contents', 'g_vault_manager->list_contents')

text = text.replace('if (!vault_path || !password) return 0;\n    try {', 'if (!vault_path || !password) return 0;\n    try {\n        if (!g_vault_manager) {\n            g_vault_manager = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");\n            if (!g_vault_manager) return 0;\n        }')

with open("core/c_api/bobfilez_c_api.cpp", "w") as f:
    f.write(text)
