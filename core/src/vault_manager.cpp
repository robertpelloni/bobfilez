/// @file vault_manager.cpp
/// @brief Implementation of the Encrypted Vault.

#include "fo/core/vault_manager.hpp"
#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <iomanip>
#ifdef FO_HAVE_OPENSSL
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/crypto.h>
#endif

#ifdef FO_HAVE_OPENSSL
namespace fo::core {

namespace {
    constexpr int AES_256_KEY_SIZE = 32;
    constexpr int AES_BLOCK_SIZE = 16;
    constexpr int GCM_IV_SIZE = 12;
    constexpr int GCM_TAG_SIZE = 16;
    constexpr int PBKDF2_ITERATIONS = 100000;
    constexpr int SALT_SIZE = 16;
    constexpr size_t CHUNK_SIZE = 4096;

    std::string generate_id() {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
        unsigned char random_bytes[4];
        RAND_bytes(random_bytes, sizeof(random_bytes));

        std::stringstream ss;
        ss << "vault_" << ms << "_";
        for (int i = 0; i < 4; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)random_bytes[i];
        }
        return ss.str();
    }
}

bool VaultManager::initialize(const std::filesystem::path& vault_path, const std::string& password) {
    vault_path_ = vault_path;
    password_ = password;
    std::filesystem::create_directories(vault_path);

    // Password verifier (".vault_verify": salt + HMAC of a known
    // constant under the PBKDF2 key). initialize() used to accept
    // ANY password, so vault-list leaked entry names/sizes to
    // whoever asked. Vaults with a verifier now reject wrong
    // passwords up front for list/lock/unlock alike.
    const auto verify_path = vault_path / ".vault_verify";
    auto hex_of = [](const unsigned char* p, size_t n) {
        std::string s;
        for (size_t i = 0; i < n; ++i) {
            char buf[3];
            std::snprintf(buf, sizeof(buf), "%02x", p[i]);
            s += buf;
        }
        return s;
    };
    auto unhex = [](const std::string& s, unsigned char* out, size_t maxn) -> size_t {
        size_t n = 0;
        for (size_t i = 0; i + 1 < s.size() && n < maxn; i += 2) {
            out[n++] = static_cast<unsigned char>(std::stoi(s.substr(i, 2), nullptr, 16));
        }
        return n;
    };
    const char* kVerifierMsg = "bobfilez-vault-verify-v1";
    auto compute_hmac = [&](const unsigned char* salt, std::string& out_hex) {
        unsigned char key[AES_256_KEY_SIZE];
        if (!derive_key(password, salt, key)) return false;
        unsigned char mac[EVP_MAX_MD_SIZE];
        unsigned int mac_len = 0;
        if (!HMAC(EVP_sha256(), key, AES_256_KEY_SIZE,
                  reinterpret_cast<const unsigned char*>(kVerifierMsg),
                  std::strlen(kVerifierMsg), mac, &mac_len)) return false;
        out_hex = hex_of(mac, mac_len);
        return true;
    };

    std::error_code ec;
    if (std::filesystem::exists(verify_path)) {
        std::ifstream in(verify_path);
        std::string salt_hex, mac_hex;
        std::getline(in, salt_hex);
        std::getline(in, mac_hex);
        unsigned char salt[SALT_SIZE];
        if (unhex(salt_hex, salt, SALT_SIZE) != SALT_SIZE) return false;
        std::string expected;
        if (!compute_hmac(salt, expected)) return false;
        if (expected.size() != mac_hex.size() ||
            CRYPTO_memcmp(expected.data(), mac_hex.data(), expected.size()) != 0) {
            return false; // wrong password (or corrupt verifier)
        }
        return true;
    }

    // No verifier yet: write one only for a fresh vault (no
    // entries). A legacy vault with entries gets none — whichever
    // password arrived first must not be able to claim it; its
    // listing stays ungated (contents remain encrypted) until
    // the owner re-inits.
    bool has_entries = false;
    for (auto& e : std::filesystem::directory_iterator(vault_path, ec)) {
        auto name = e.path().filename().string();
        if (name.rfind("vault_", 0) == 0) { has_entries = true; break; }
    }
    if (!has_entries) {
        unsigned char salt[SALT_SIZE];
        if (RAND_bytes(salt, sizeof(salt)) == 1) {
            std::string mac_hex;
            if (compute_hmac(salt, mac_hex)) {
                std::ofstream out(verify_path);
                out << hex_of(salt, SALT_SIZE) << "\n" << mac_hex << "\n";
            }
        }
    }
    return true;
}

bool VaultManager::derive_key(const std::string& password, const unsigned char* salt, unsigned char* key) const {
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                          salt, SALT_SIZE, PBKDF2_ITERATIONS,
                          EVP_sha256(), AES_256_KEY_SIZE, key) != 1) {
        return false;
    }
    return true;
}

bool VaultManager::encrypt_file(const std::filesystem::path& input, const std::filesystem::path& output) const {
    std::ifstream in(input, std::ios::binary);
    if (!in) return false;

    // Write to a temp sibling and rename into place only on success,
    // so a failed encrypt never leaves a partial vault entry behind.
    std::filesystem::path tmp = output;
    tmp += ".part";
    auto fail = [&]() {
        std::error_code ec;
        std::filesystem::remove(tmp, ec);
        return false;
    };

    std::ofstream out(tmp, std::ios::binary);
    if (!out) return false;

    unsigned char salt[SALT_SIZE];
    unsigned char iv[GCM_IV_SIZE];
    unsigned char key[AES_256_KEY_SIZE];

    if (RAND_bytes(salt, sizeof(salt)) != 1) return fail();
    if (RAND_bytes(iv, sizeof(iv)) != 1) return fail();

    if (!derive_key(password_, salt, key)) return fail();

    out.write(reinterpret_cast<char*>(salt), sizeof(salt));
    out.write(reinterpret_cast<char*>(iv), sizeof(iv));

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return fail();

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        return fail();
    }

    if (1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return fail();
    }

    unsigned char in_buf[CHUNK_SIZE];
    unsigned char out_buf[CHUNK_SIZE + AES_BLOCK_SIZE];
    int out_len;

    while (in.read(reinterpret_cast<char*>(in_buf), CHUNK_SIZE) || in.gcount() > 0) {
        if (1 != EVP_EncryptUpdate(ctx, out_buf, &out_len, in_buf, in.gcount())) {
            EVP_CIPHER_CTX_free(ctx);
            return fail();
        }
        out.write(reinterpret_cast<char*>(out_buf), out_len);
    }

    if (1 != EVP_EncryptFinal_ex(ctx, out_buf, &out_len)) {
        EVP_CIPHER_CTX_free(ctx);
        return fail();
    }
    out.write(reinterpret_cast<char*>(out_buf), out_len);

    unsigned char tag[GCM_TAG_SIZE];
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_SIZE, tag)) {
        EVP_CIPHER_CTX_free(ctx);
        return fail();
    }
    out.write(reinterpret_cast<char*>(tag), sizeof(tag));

    EVP_CIPHER_CTX_free(ctx);
    out.close();

    std::error_code rec_ec;
    std::filesystem::rename(tmp, output, rec_ec);
    if (rec_ec) return fail();

    // Save metadata
    std::filesystem::path meta_path = output;
    meta_path += ".meta";
    std::ofstream meta_out(meta_path);
    if (meta_out) {
        meta_out << input.filename().string() << "\n";
    }

    return true;
}

bool VaultManager::decrypt_file(const std::filesystem::path& input, const std::filesystem::path& output) const {
    std::ifstream in(input, std::ios::binary);
    if (!in) return false;

    unsigned char salt[SALT_SIZE];
    unsigned char iv[GCM_IV_SIZE];

    if (!in.read(reinterpret_cast<char*>(salt), sizeof(salt))) return false;
    if (!in.read(reinterpret_cast<char*>(iv), sizeof(iv))) return false;

    unsigned char key[AES_256_KEY_SIZE];
    if (!derive_key(password_, salt, key)) return false;

    // Read the file to memory or get its size to extract the tag at the end
    in.seekg(0, std::ios::end);
    std::streampos file_size = in.tellg();
    if (file_size < static_cast<std::streampos>(SALT_SIZE + GCM_IV_SIZE + GCM_TAG_SIZE)) {
        return false; // File too small
    }

    std::streampos data_size = file_size - static_cast<std::streampos>(SALT_SIZE + GCM_IV_SIZE + GCM_TAG_SIZE);

    in.seekg(file_size - static_cast<std::streampos>(GCM_TAG_SIZE));
    unsigned char tag[GCM_TAG_SIZE];
    if (!in.read(reinterpret_cast<char*>(tag), sizeof(tag))) return false;

    in.seekg(SALT_SIZE + GCM_IV_SIZE);

    // Decrypt to a temp sibling and rename into place only after the
    // GCM tag verifies: the old code streamed unauthenticated
    // plaintext straight to the destination, so a wrong password
    // left a garbage "restored" file behind.
    std::filesystem::path tmp = output;
    tmp += ".part";
    auto fail = [&]() {
        std::error_code ec;
        std::filesystem::remove(tmp, ec);
        return false;
    };

    std::ofstream out(tmp, std::ios::binary);
    if (!out) return false;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return fail();

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        return fail();
    }

    if (1 != EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return fail();
    }

    unsigned char in_buf[CHUNK_SIZE];
    unsigned char out_buf[CHUNK_SIZE + AES_BLOCK_SIZE];
    int out_len;

    std::streampos remaining = data_size;
    while (remaining > 0) {
        std::streamsize to_read = (remaining > static_cast<std::streampos>(CHUNK_SIZE)) ? CHUNK_SIZE : static_cast<std::streamsize>(remaining);
        if (!in.read(reinterpret_cast<char*>(in_buf), to_read)) {
             EVP_CIPHER_CTX_free(ctx);
             return fail();
        }
        if (1 != EVP_DecryptUpdate(ctx, out_buf, &out_len, in_buf, to_read)) {
            EVP_CIPHER_CTX_free(ctx);
            return fail();
        }
        out.write(reinterpret_cast<char*>(out_buf), out_len);
        remaining -= to_read;
    }

    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, GCM_TAG_SIZE, tag)) {
        EVP_CIPHER_CTX_free(ctx);
        return fail();
    }

    int ret = EVP_DecryptFinal_ex(ctx, out_buf, &out_len);
    EVP_CIPHER_CTX_free(ctx);

    if (ret > 0) {
        out.write(reinterpret_cast<char*>(out_buf), out_len);
        out.close();
        std::error_code rec_ec;
        std::filesystem::rename(tmp, output, rec_ec);
        if (rec_ec) return fail();
        return true;
    } else {
        out.close();
        return fail(); // Decryption failed (e.g. wrong password or corrupted data)
    }
}

bool VaultManager::lock_file(const std::filesystem::path& file_path) {
    if (!std::filesystem::exists(file_path)) return false;
    
    std::string vault_id = generate_id();
    std::filesystem::path dest_path = vault_path_ / vault_id;

    if (encrypt_file(file_path, dest_path)) {
        // Only delete if encryption succeeds
        // In a real scenario we'd do secure delete (wiping)
        std::filesystem::remove(file_path);
        // std::cout << "[Vault] Encrypting and locking: " << file_path << " -> " << vault_id << "\n";
        return true;
    }

    return false;
}

bool VaultManager::unlock_file(const std::string& vault_id, const std::filesystem::path& dest_path) {
    std::filesystem::path src_path = vault_path_ / vault_id;
    if (!std::filesystem::exists(src_path)) return false;

    std::filesystem::path actual_dest = dest_path;

    // Check if dest_path is a directory. If so, append original filename if we have it
    if (std::filesystem::is_directory(dest_path)) {
        std::filesystem::path meta_path = src_path;
        meta_path += ".meta";
        if (std::filesystem::exists(meta_path)) {
            std::ifstream meta_in(meta_path);
            std::string orig_name;
            if (std::getline(meta_in, orig_name)) {
                actual_dest = dest_path / orig_name;
            }
        }
    }

    // Never silently replace an existing destination file: the
    // decrypt rename would overwrite its bytes. Refuse instead;
    // the vault entry survives for a retry with a free path.
    if (std::filesystem::exists(actual_dest)) return false;

    if (decrypt_file(src_path, actual_dest)) {
        std::filesystem::remove(src_path);

        std::filesystem::path meta_path = src_path;
        meta_path += ".meta";
        if (std::filesystem::exists(meta_path)) {
            std::filesystem::remove(meta_path);
        }

        // std::cout << "[Vault] Decrypting and unlocking to: " << actual_dest << "\n";
        return true;
    }

    return false;
}

std::vector<VaultEntry> VaultManager::list_contents() {
    std::vector<VaultEntry> entries;
    if (!std::filesystem::exists(vault_path_)) return entries;

    for (const auto& entry : std::filesystem::directory_iterator(vault_path_)) {
        if (entry.is_regular_file()) {
            std::string id = entry.path().filename().string();

            // Skip .meta files from the main list, we'll read them when we find the main file
            if (id.length() > 5 && id.substr(id.length() - 5) == ".meta") {
                continue;
            }
            // Skip vault bookkeeping (the password verifier) and
            // temp residue: only generated vault_<id> entries are
            // contents. The verifier briefly listed itself here.
            if (!id.empty() && id[0] == '.') continue;
            if (id.length() > 5 && id.substr(id.length() - 5) == ".part") continue;

            VaultEntry ve;
            ve.id = id;
            ve.size = entry.file_size();

            // Try reading original name
            std::filesystem::path meta_path = entry.path();
            meta_path += ".meta";
            if (std::filesystem::exists(meta_path)) {
                std::ifstream meta_in(meta_path);
                std::getline(meta_in, ve.original_name);
            } else {
                ve.original_name = "Unknown";
            }

            auto ftime = std::filesystem::last_write_time(entry);
            ve.added_at = std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count();

            entries.push_back(ve);
        }
    }

    return entries;
}

} // namespace fo::core
#else
namespace fo::core {
// Built without OpenSSL: vault operations are unavailable (FO_HAVE_OPENSSL unset).
bool VaultManager::initialize(const std::filesystem::path&, const std::string&) { return false; }
bool VaultManager::lock_file(const std::filesystem::path&) { return false; }
bool VaultManager::unlock_file(const std::string&, const std::filesystem::path&) { return false; }
std::vector<VaultEntry> VaultManager::list_contents() { return {}; }
} // namespace fo::core
#endif
