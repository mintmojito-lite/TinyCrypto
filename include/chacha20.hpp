#pragma once

#include <cstdint>
#include <cstddef>

namespace tinycrypto {

class ChaCha20 {
public:
    ChaCha20(const uint8_t key[32], const uint8_t nonce[12], uint32_t counter = 0);

    void encrypt(const uint8_t* in, uint8_t* out, size_t len);
    void decrypt(const uint8_t* in, uint8_t* out, size_t len);

    void encrypt_scalar(const uint8_t* in, uint8_t* out, size_t len);
#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
    void encrypt_avx2(const uint8_t* in, uint8_t* out, size_t len);
#endif

private:
    uint32_t m_state[16]; // Initial permutation state setup
};

} // namespace tinycrypto
