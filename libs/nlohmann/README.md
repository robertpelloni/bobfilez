# vendored nlohmann/json single header (NOT a submodule)

json.hpp v3.11.3 (https://github.com/nlohmann/json/releases/tag/v3.11.3), fetched 2026-11-03.
Why vendored: core audit_logger.cpp needs <nlohmann/json.hpp> unconditionally; vcpkg (which provides it per vcpkg.json) is absent on no-admin hosts. Resolves via the existing fo_core PRIVATE ${LIBS_DIR} include path.
