with open("tests/test_vault_manager.cpp", "r") as f:
    text = f.read()

text = text.replace('#include "fo/core/vault_manager.hpp"', '#include "fo/core/vault_manager.hpp"\n#include "fo/core/registry.hpp"')

with open("tests/test_vault_manager.cpp", "w") as f:
    f.write(text)
