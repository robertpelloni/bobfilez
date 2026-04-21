import re

with open("tests/test_vault_manager.cpp", "r") as f:
    text = f.read()

# Ah, the problem is `Registry::instance().create("aes256_gcm")` is returning a nullptr
# because `register_all_providers()` hasn't been called in the test suite!
# The tests initialize individual classes but don't call the global register.
text = text.replace('class VaultManagerTest : public ::testing::Test {', 'extern void force_register_vault_manager();\nclass VaultManagerTest : public ::testing::Test {\npublic:\n    VaultManagerTest() { fo::core::providers::force_register_vault_manager(); }')

with open("tests/test_vault_manager.cpp", "w") as f:
    f.write(text)
