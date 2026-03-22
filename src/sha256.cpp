#include "sha256.hpp"
#include "utils.hpp"
#include <cstring>

namespace tinycrypto {

using namespace utils;

namespace {

const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ ((~x) & z);
}

inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

inline uint32_t ep0(uint32_t x) {
    return rotr32(x, 2) ^ rotr32(x, 13) ^ rotr32(x, 22);
}

inline uint32_t ep1(uint32_t x) {
    return rotr32(x, 6) ^ rotr32(x, 11) ^ rotr32(x, 25);
}

inline uint32_t sig0(uint32_t x) {
    return rotr32(x, 7) ^ rotr32(x, 18) ^ (x >> 3);
}

inline uint32_t sig1(uint32_t x) {
    return rotr32(x, 17) ^ rotr32(x, 19) ^ (x >> 10);
}

} // namespace

SHA256::SHA256() {
    m_state[0] = 0x6a09e667;
    m_state[1] = 0xbb67ae85;
    m_state[2] = 0x3c6ef372;
    m_state[3] = 0xa54ff53a;
    m_state[4] = 0x510e527f;
    m_state[5] = 0x9b05688c;
    m_state[6] = 0x1f83d9ab;
    m_state[7] = 0x5be0cd19;
    m_total_len = 0;
}

void SHA256::transform() {
    uint32_t W[64];
    for (int i = 0; i < 16; ++i) {
        uint32_t word;
        std::memcpy(&word, m_buffer + i * 4, 4);
        W[i] = bswap32(word);
    }
    for (int i = 16; i < 64; ++i) {
        W[i] = sig1(W[i - 2]) + W[i - 7] + sig0(W[i - 15]) + W[i - 16];
    }

    uint32_t a = m_state[0];
    uint32_t b = m_state[1];
    uint32_t c = m_state[2];
    uint32_t d = m_state[3];
    uint32_t e = m_state[4];
    uint32_t f = m_state[5];
    uint32_t g = m_state[6];
    uint32_t h = m_state[7];

    for (int i = 0; i < 64; ++i) {
        uint32_t temp1 = h + ep1(e) + ch(e, f, g) + K[i] + W[i];
        uint32_t temp2 = ep0(a) + maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    m_state[0] += a;
    m_state[1] += b;
    m_state[2] += c;
    m_state[3] += d;
    m_state[4] += e;
    m_state[5] += f;
    m_state[6] += g;
    m_state[7] += h;

    // Erase intermediate state
    secure_zero(W, sizeof(W));
}

void SHA256::update(const void* data, size_t len) {
    const uint8_t* p = static_cast<const uint8_t*>(data);
    size_t index = m_total_len % BLOCK_SIZE;
    m_total_len += len;

    while (len > 0) {
        size_t left = BLOCK_SIZE - index;
        size_t fill = (len < left) ? len : left;

        std::memcpy(m_buffer + index, p, fill);
        len -= fill;
        p += fill;
        index += fill;

        if (index == BLOCK_SIZE) {
            transform();
            index = 0;
        }
    }
}

void SHA256::finalize(uint8_t hash[HASH_SIZE]) {
    size_t index = m_total_len % BLOCK_SIZE;
    m_buffer[index++] = 0x80;

    if (index > 56) {
        std::memset(m_buffer + index, 0, BLOCK_SIZE - index);
        transform();
        index = 0;
    }

    std::memset(m_buffer + index, 0, 56 - index);

    // Padding total length in bits as a 64-bit big-endian integer
    uint64_t bits = bswap64(m_total_len * 8);
    std::memcpy(m_buffer + 56, &bits, 8);

    transform();

    for (int i = 0; i < 8; ++i) {
        uint32_t word = bswap32(m_state[i]);
        std::memcpy(hash + i * 4, &word, 4);
    }

    // Clean sensitive internal state
    secure_zero(m_state, sizeof(m_state));
    secure_zero(m_buffer, sizeof(m_buffer));
}

std::array<uint8_t, SHA256::HASH_SIZE> SHA256::hash(const void* data, size_t len) {
    SHA256 ctx;
    std::array<uint8_t, HASH_SIZE> res;
    ctx.update(data, len);
    ctx.finalize(res.data());
    return res;
}

} // namespace tinycrypto
