with open("core/src/providers/register_all_providers.cpp", "r") as f:
    text = f.read()

text = text.replace('} // namespace fo::core::providers', '    void force_register_vault_manager();\n} // namespace fo::core::providers')

text = text.replace('}\n\n} // namespace fo::core', '    force_register_vault_manager();\n}\n\n} // namespace fo::core')

with open("core/src/providers/register_all_providers.cpp", "w") as f:
    f.write(text)
