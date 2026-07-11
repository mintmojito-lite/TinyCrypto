#pragma once

#include <cstdint>
#include <cstddef>

// ============================================================================
// Secure Memory Erasure
// ============================================================================

#include "aes256.hpp"
#include <cstring>

// explicitly scrubs sensitive key material from memory (and registers if inlined)
// preventing it from being optimized out by the compiler.
inline void explicit_bzero(void* ptr, size_t len) {
    if (ptr == nullptr) return;
    
    volatile uint8_t* p = static_cast<volatile uint8_t*>(ptr);
    while (len--) {
        *p++ = 0;
    }
    // Compiler barrier to ensure memory operations are not reordered or optimized away
#if !defined(_MSC_VER)
    asm volatile("" : : "r"(ptr) : "memory");
#endif
}

// ============================================================================
// Stack-Allocated Cryptographic Contexts and Placeholders
// Zero Dynamic Memory / Heap Allocations
// ============================================================================

namespace tinycrypto {

// AES-256-CBC
struct AES256_CBC_Context {
    uint32_t round_keys[60];
    uint8_t iv[16];
};

inline void aes256_cbc_init(AES256_CBC_Context* ctx, const uint8_t* key, const uint8_t* iv) {
    std::array<uint8_t, 32> k;
    std::memcpy(k.data(), key, 32);
    aes256_init(*reinterpret_cast<Aes256Ctx*>(&ctx->round_keys), k);
    std::memcpy(ctx->iv, iv, 16);
    explicit_bzero(k.data(), 32);
}

inline void aes256_cbc_encrypt(AES256_CBC_Context* ctx, const uint8_t* plaintext, uint8_t* ciphertext, size_t blocks) {
    const Aes256Ctx& aes_ctx = *reinterpret_cast<const Aes256Ctx*>(&ctx->round_keys);
    for (size_t b = 0; b < blocks; ++b) {
        for (int i = 0; i < 16; ++i) {
            ctx->iv[i] ^= plaintext[b * 16 + i];
        }
        aes256_encrypt_block(aes_ctx, ctx->iv, ctx->iv);
        for (int i = 0; i < 16; ++i) {
            ciphertext[b * 16 + i] = ctx->iv[i];
        }
    }
}




} // namespace tinycrypto
