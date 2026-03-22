#pragma once
#include <array>
#include <cstdint>
#include <cstddef>

namespace tinycrypto {

struct Aes256Ctx {
    // 14 Rounds for AES-256 + 1 initial key = 15 total Round Keys.
    // Each Round Key is exactly 16 bytes (128 bits) corresponding to the block size mapping constraint.
    std::array<std::array<uint8_t, 16>, 15> round_keys;
};

// Generates expanded Key Schedule sequentially mapping derivations using RCON evaluations
void aes256_init(Aes256Ctx& ctx, const std::array<uint8_t, 32>& key);

// Encrypts exactly one 16-byte block natively mapping to internal structural arrays
void aes256_encrypt_block(const Aes256Ctx& ctx, const uint8_t* in, uint8_t* out);

// Counter streaming implementation relying structurally universally upon encryption layouts securely wrapping constants
void aes256_ctr_encrypt(const Aes256Ctx& ctx, const std::array<uint8_t, 16>& nonce_and_initial_counter, const uint8_t* in, uint8_t* out, size_t len);

} // namespace tinycrypto
