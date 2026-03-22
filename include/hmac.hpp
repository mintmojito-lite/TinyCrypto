#pragma once

#include "sha256.hpp"
#include <array>

namespace tinycrypto {

class HMAC_SHA256 {
public:
    static constexpr size_t HASH_SIZE = SHA256::HASH_SIZE;
    
    // One-shot HMAC-SHA256 definition according to FIPS 198-1
    static std::array<uint8_t, HASH_SIZE> mac(const uint8_t* key, size_t key_len, const uint8_t* msg, size_t msg_len);
    
    // Constant-time validation
    static bool verify(const uint8_t* key, size_t key_len, const uint8_t* msg, size_t msg_len, const uint8_t expected_mac[HASH_SIZE]);
};

} // namespace tinycrypto
