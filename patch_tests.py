import re

with open("tests/test_vault_manager.cpp", "r") as f:
    text = f.read()

text = text.replace('VaultManager vm;', 'auto vm = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");')
text = text.replace('VaultManager vm2;', 'auto vm2 = fo::core::Registry<fo::core::IVaultManager>::instance().create("aes256_gcm");')
text = text.replace('vm.', 'vm->')
text = text.replace('vm2.', 'vm2->')

with open("tests/test_vault_manager.cpp", "w") as f:
    f.write(text)
