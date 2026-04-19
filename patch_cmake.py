with open("core/CMakeLists.txt", "r") as f:
    text = f.read()

text = text.replace('${CMAKE_CURRENT_SOURCE_DIR}/src/vault_manager.cpp', '${CMAKE_CURRENT_SOURCE_DIR}/src/vault_manager.cpp\n    ${CMAKE_CURRENT_SOURCE_DIR}/src/providers/vault_manager_reg.cpp')

with open("core/CMakeLists.txt", "w") as f:
    f.write(text)
