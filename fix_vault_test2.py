with open("tests/test_vault_manager.cpp", "r") as f:
    text = f.read()

text = text.replace('extern void force_register_vault_manager();', 'namespace fo::core::providers { extern void force_register_vault_manager(); }')

with open("tests/test_vault_manager.cpp", "w") as f:
    f.write(text)
