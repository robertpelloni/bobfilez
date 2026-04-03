#pragma once
/// @file omnicrypt_interface.hpp
/// @brief Advanced Cryptography and Steganography Engine for bobfilez.
///
/// OmniCrypt provides military-grade encryption (libsodium: XChaCha20-Poly1305)
/// and LSB (Least Significant Bit) steganography (opencv) to hide sensitive 
/// payloads inside innocent-looking carrier files (images/audio/video), 
/// providing plausible deniability.

#include <string>
#include <vector>
#include <filesystem>
#include <memory>

namespace fo::core {

struct CryptResult {
    bool success;
    std::filesystem::path output_path;
    std::string error_message;
    double processing_time_sec;
    size_t payload_size_bytes;
};

struct StegoCapacity {
    size_t max_payload_bytes;
    bool is_viable_carrier;
};

class IOmniCryptEngine {
public:
    virtual ~IOmniCryptEngine() = default;

    /// Encrypt a file using libsodium (XChaCha20-Poly1305)
    virtual CryptResult encrypt_file(const std::filesystem::path& input, const std::string& password) = 0;

    /// Decrypt a standard encrypted file
    virtual CryptResult decrypt_file(const std::filesystem::path& input, const std::string& password) = 0;

    /// Calculate how much hidden data a carrier image/video can hold
    virtual StegoCapacity calculate_stego_capacity(const std::filesystem::path& carrier) = 0;

    /// Hide an encrypted payload inside a carrier image (Steganography)
    virtual CryptResult embed_payload(const std::filesystem::path& carrier, 
                                      const std::filesystem::path& payload, 
                                      const std::string& password) = 0;

    /// Extract and decrypt a payload from a carrier image
    virtual CryptResult extract_payload(const std::filesystem::path& carrier, 
                                        const std::filesystem::path& output_dir, 
                                        const std::string& password) = 0;
};

} // namespace fo::core
