import re

with open("core/include/fo/core/vault_manager.hpp", "r") as f:
    text = f.read()

text = text.replace('#include <vector>\n', '#include <vector>\n#include "fo/core/interfaces.hpp"\n')
text = text.replace('class VaultManager {', 'class VaultManager : public IVaultManager {')

text = text.replace('    bool initialize(const std::filesystem::path& vault_path, const std::string& password);', '    bool initialize(const std::filesystem::path& vault_path, const std::string& password) override;')
text = text.replace('    bool lock_file(const std::filesystem::path& file_path);', '    bool lock_file(const std::filesystem::path& file_path) override;')
text = text.replace('    bool unlock_file(const std::string& vault_id, const std::filesystem::path& dest_path);', '    bool unlock_file(const std::string& vault_id, const std::filesystem::path& dest_path) override;')

# Remove struct VaultEntry definition from here
text = re.sub(r'    struct VaultEntry {.*?};\n', '', text, flags=re.DOTALL)

text = text.replace('    std::vector<VaultEntry> list_contents();', '    std::vector<VaultEntry> list_contents() override;')

with open("core/include/fo/core/vault_manager.hpp", "w") as f:
    f.write(text)
