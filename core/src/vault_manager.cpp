/// @file vault_manager.cpp
/// @brief Implementation of the Encrypted Vault.

#include "fo/core/vault_manager.hpp"
#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/rand.h>

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
    // std::cout << "[Vault] Initialized at " << vault_path << "\n";
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

    std::ofstream out(output, std::ios::binary);
    if (!out) return false;

    unsigned char salt[SALT_SIZE];
    unsigned char iv[GCM_IV_SIZE];
    unsigned char key[AES_256_KEY_SIZE];

    if (RAND_bytes(salt, sizeof(salt)) != 1) return false;
    if (RAND_bytes(iv, sizeof(iv)) != 1) return false;

    if (!derive_key(password_, salt, key)) return false;

    out.write(reinterpret_cast<char*>(salt), sizeof(salt));
    out.write(reinterpret_cast<char*>(iv), sizeof(iv));

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    if (1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    unsigned char in_buf[CHUNK_SIZE];
    unsigned char out_buf[CHUNK_SIZE + AES_BLOCK_SIZE];
    int out_len;

    while (in.read(reinterpret_cast<char*>(in_buf), CHUNK_SIZE) || in.gcount() > 0) {
        if (1 != EVP_EncryptUpdate(ctx, out_buf, &out_len, in_buf, in.gcount())) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        out.write(reinterpret_cast<char*>(out_buf), out_len);
    }

    if (1 != EVP_EncryptFinal_ex(ctx, out_buf, &out_len)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    out.write(reinterpret_cast<char*>(out_buf), out_len);

    unsigned char tag[GCM_TAG_SIZE];
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_SIZE, tag)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }
    out.write(reinterpret_cast<char*>(tag), sizeof(tag));

    EVP_CIPHER_CTX_free(ctx);

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

    std::ofstream out(output, std::ios::binary);
    if (!out) return false;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    if (1 != EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    unsigned char in_buf[CHUNK_SIZE];
    unsigned char out_buf[CHUNK_SIZE + AES_BLOCK_SIZE];
    int out_len;

    std::streampos remaining = data_size;
    while (remaining > 0) {
        std::streamsize to_read = (remaining > static_cast<std::streampos>(CHUNK_SIZE)) ? CHUNK_SIZE : static_cast<std::streamsize>(remaining);
        if (!in.read(reinterpret_cast<char*>(in_buf), to_read)) {
             EVP_CIPHER_CTX_free(ctx);
             return false;
        }
        if (1 != EVP_DecryptUpdate(ctx, out_buf, &out_len, in_buf, to_read)) {
            EVP_CIPHER_CTX_free(ctx);
            return false;
        }
        out.write(reinterpret_cast<char*>(out_buf), out_len);
        remaining -= to_read;
    }

    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, GCM_TAG_SIZE, tag)) {
        EVP_CIPHER_CTX_free(ctx);
        return false;
    }

    int ret = EVP_DecryptFinal_ex(ctx, out_buf, &out_len);
    EVP_CIPHER_CTX_free(ctx);

    if (ret > 0) {
        out.write(reinterpret_cast<char*>(out_buf), out_len);
        return true;
    } else {
        return false; // Decryption failed (e.g. wrong password or corrupted data)
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
