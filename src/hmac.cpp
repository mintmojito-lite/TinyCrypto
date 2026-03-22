#include "hmac.hpp"
#include "utils.hpp"
#include <cstring>

namespace tinycrypto {

std::array<uint8_t, HMAC_SHA256::HASH_SIZE> HMAC_SHA256::mac(const uint8_t* key, size_t key_len, const uint8_t* msg, size_t msg_len) {
    uint8_t K[SHA256::BLOCK_SIZE] = {0};

    if (key_len > SHA256::BLOCK_SIZE) {
        auto hash = SHA256::hash(key, key_len);
        std::memcpy(K, hash.data(), hash.size());
    } else {
        std::memcpy(K, key, key_len);
    }

    uint8_t ipad[SHA256::BLOCK_SIZE];
    uint8_t opad[SHA256::BLOCK_SIZE];

    for (size_t i = 0; i < SHA256::BLOCK_SIZE; ++i) {
        ipad[i] = K[i] ^ 0x36;
        opad[i] = K[i] ^ 0x5c;
    }

    // Inner hash: SHA256(K ^ ipad || msg)
    SHA256 inner;
    inner.update(ipad, SHA256::BLOCK_SIZE);
    inner.update(msg, msg_len);
    std::array<uint8_t, SHA256::HASH_SIZE> inner_hash;
    inner.finalize(inner_hash.data());

    // Outer hash: SHA256(K ^ opad || inner_hash)
    SHA256 outer;
    outer.update(opad, SHA256::BLOCK_SIZE);
    outer.update(inner_hash.data(), inner_hash.size());
    std::array<uint8_t, SHA256::HASH_SIZE> mac_hash;
    outer.finalize(mac_hash.data());

    utils::secure_zero(K, sizeof(K));
    utils::secure_zero(ipad, sizeof(ipad));
    utils::secure_zero(opad, sizeof(opad));

    return mac_hash;
}

bool HMAC_SHA256::verify(const uint8_t* key, size_t key_len, const uint8_t* msg, size_t msg_len, const uint8_t expected_mac[HASH_SIZE]) {
    auto generated = mac(key, key_len, msg, msg_len);
    return utils::constant_time_equal(generated.data(), expected_mac, HASH_SIZE);
}

} // namespace tinycrypto
