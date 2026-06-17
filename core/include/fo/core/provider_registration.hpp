#pragma once

namespace fo::core {
void register_scanner_std();
void register_scanner_win32();
void register_scanner_dirent();
void register_hasher_fast64();
void register_hasher_sha256();
void register_hasher_xxhash();
void register_hasher_blake3();
void register_metadata_tinyexif();
void register_linter_std();
void register_autonomous_sync_service();
void register_autonomous_dev_protocol();

void register_all_providers();
}
