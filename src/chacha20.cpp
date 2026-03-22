#include "chacha20.hpp"
#include "utils.hpp"
#include <cstring>

#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
#endif

namespace tinycrypto {

using namespace utils;

namespace {

inline uint32_t load32_le(const uint8_t* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}

inline void store32_le(uint8_t* p, uint32_t v) {
    p[0] = static_cast<uint8_t>(v);
    p[1] = static_cast<uint8_t>(v >> 8);
    p[2] = static_cast<uint8_t>(v >> 16);
    p[3] = static_cast<uint8_t>(v >> 24);
}

#define QR(a, b, c, d) \
    a += b; d ^= a; d = rotl32(d, 16); \
    c += d; b ^= c; b = rotl32(b, 12); \
    a += b; d ^= a; d = rotl32(d, 8); \
    c += d; b ^= c; b = rotl32(b, 7);

#define QRAVX2_INTRA(A, B, C, D) \
    A = _mm256_add_epi32(A, B); D = _mm256_xor_si256(D, A); \
    D = _mm256_or_si256(_mm256_slli_epi32(D, 16), _mm256_srli_epi32(D, 16)); \
    \
    C = _mm256_add_epi32(C, D); B = _mm256_xor_si256(B, C); \
    B = _mm256_or_si256(_mm256_slli_epi32(B, 12), _mm256_srli_epi32(B, 20)); \
    \
    A = _mm256_add_epi32(A, B); D = _mm256_xor_si256(D, A); \
    D = _mm256_or_si256(_mm256_slli_epi32(D, 8), _mm256_srli_epi32(D, 24)); \
    \
    C = _mm256_add_epi32(C, D); B = _mm256_xor_si256(B, C); \
    B = _mm256_or_si256(_mm256_slli_epi32(B, 7), _mm256_srli_epi32(B, 25));

} // namespace

ChaCha20::ChaCha20(const uint8_t key[32], const uint8_t nonce[12], uint32_t counter) {
    m_state[0] = 0x61707865;
    m_state[1] = 0x3320646e;
    m_state[2] = 0x79622d32;
    m_state[3] = 0x6b206574;

    for (int i = 0; i < 8; ++i) {
        m_state[4 + i] = load32_le(key + i * 4);
    }

    m_state[12] = counter;
    for (int i = 0; i < 3; ++i) {
        m_state[13 + i] = load32_le(nonce + i * 4);
    }
}

void ChaCha20::encrypt_scalar(const uint8_t* in, uint8_t* out, size_t len) {
    uint32_t state[16];
    uint8_t block[64];

    while (len > 0) {
        for (int i = 0; i < 16; ++i) {
            state[i] = m_state[i];
        }

        for (int i = 0; i < 10; ++i) {
            QR(state[0], state[4], state[8], state[12]);
            QR(state[1], state[5], state[9], state[13]);
            QR(state[2], state[6], state[10], state[14]);
            QR(state[3], state[7], state[11], state[15]);

            QR(state[0], state[5], state[10], state[15]);
            QR(state[1], state[6], state[11], state[12]);
            QR(state[2], state[7], state[8], state[13]);
            QR(state[3], state[4], state[9], state[14]);
        }

        for (int i = 0; i < 16; ++i) {
            store32_le(block + i * 4, state[i] + m_state[i]);
        }

        size_t chunk = (len < 64) ? len : 64;
        for (size_t i = 0; i < chunk; ++i) {
            out[i] = in[i] ^ block[i];
        }

        m_state[12]++;
        in += chunk;
        out += chunk;
        len -= chunk;
    }
    
    secure_zero(state, sizeof(state));
    secure_zero(block, sizeof(block));
}

#if defined(__x86_64__) || defined(_M_X64)
void ChaCha20::encrypt_avx2(const uint8_t* in, uint8_t* out, size_t len) {
    uint8_t block[256];

    while (len >= 256) {
        // Load four blocks using 8 AVX2 registers. R0-R3 handle block 0 & 1. R4-R7 handle block 2 & 3.
        __m256i R0 = _mm256_set_epi32(m_state[3], m_state[2], m_state[1], m_state[0], m_state[3], m_state[2], m_state[1], m_state[0]);
        __m256i R1 = _mm256_set_epi32(m_state[7], m_state[6], m_state[5], m_state[4], m_state[7], m_state[6], m_state[5], m_state[4]);
        __m256i R2 = _mm256_set_epi32(m_state[11], m_state[10], m_state[9], m_state[8], m_state[11], m_state[10], m_state[9], m_state[8]);
        __m256i R3 = _mm256_set_epi32(m_state[15], m_state[14], m_state[13], m_state[12]+1, m_state[15], m_state[14], m_state[13], m_state[12]);

        __m256i R4 = _mm256_set_epi32(m_state[3], m_state[2], m_state[1], m_state[0], m_state[3], m_state[2], m_state[1], m_state[0]);
        __m256i R5 = _mm256_set_epi32(m_state[7], m_state[6], m_state[5], m_state[4], m_state[7], m_state[6], m_state[5], m_state[4]);
        __m256i R6 = _mm256_set_epi32(m_state[11], m_state[10], m_state[9], m_state[8], m_state[11], m_state[10], m_state[9], m_state[8]);
        __m256i R7 = _mm256_set_epi32(m_state[15], m_state[14], m_state[13], m_state[12]+3, m_state[15], m_state[14], m_state[13], m_state[12]+2);

        __m256i O0 = R0, O1 = R1, O2 = R2, O3 = R3;
        __m256i O4 = R4, O5 = R5, O6 = R6, O7 = R7;

        for (int i = 0; i < 10; ++i) {
            QRAVX2_INTRA(R0, R1, R2, R3);
            QRAVX2_INTRA(R4, R5, R6, R7);
            
            R1 = _mm256_shuffle_epi32(R1, _MM_SHUFFLE(0, 3, 2, 1));
            R2 = _mm256_shuffle_epi32(R2, _MM_SHUFFLE(1, 0, 3, 2));
            R3 = _mm256_shuffle_epi32(R3, _MM_SHUFFLE(2, 1, 0, 3));
            
            R5 = _mm256_shuffle_epi32(R5, _MM_SHUFFLE(0, 3, 2, 1));
            R6 = _mm256_shuffle_epi32(R6, _MM_SHUFFLE(1, 0, 3, 2));
            R7 = _mm256_shuffle_epi32(R7, _MM_SHUFFLE(2, 1, 0, 3));

            QRAVX2_INTRA(R0, R1, R2, R3);
            QRAVX2_INTRA(R4, R5, R6, R7);

            R1 = _mm256_shuffle_epi32(R1, _MM_SHUFFLE(2, 1, 0, 3));
            R2 = _mm256_shuffle_epi32(R2, _MM_SHUFFLE(1, 0, 3, 2));
            R3 = _mm256_shuffle_epi32(R3, _MM_SHUFFLE(0, 3, 2, 1));

            R5 = _mm256_shuffle_epi32(R5, _MM_SHUFFLE(2, 1, 0, 3));
            R6 = _mm256_shuffle_epi32(R6, _MM_SHUFFLE(1, 0, 3, 2));
            R7 = _mm256_shuffle_epi32(R7, _MM_SHUFFLE(0, 3, 2, 1));
        }

        R0 = _mm256_add_epi32(R0, O0);
        R1 = _mm256_add_epi32(R1, O1);
        R2 = _mm256_add_epi32(R2, O2);
        R3 = _mm256_add_epi32(R3, O3);
        
        R4 = _mm256_add_epi32(R4, O4);
        R5 = _mm256_add_epi32(R5, O5);
        R6 = _mm256_add_epi32(R6, O6);
        R7 = _mm256_add_epi32(R7, O7);

        // De-interleave and write directly to buffer
        _mm256_storeu_si256((__m256i*)(block + 0), _mm256_permute2x128_si256(R0, R1, 0x20));
        _mm256_storeu_si256((__m256i*)(block + 32), _mm256_permute2x128_si256(R2, R3, 0x20));
        
        _mm256_storeu_si256((__m256i*)(block + 64), _mm256_permute2x128_si256(R0, R1, 0x31));
        _mm256_storeu_si256((__m256i*)(block + 96), _mm256_permute2x128_si256(R2, R3, 0x31));
        
        _mm256_storeu_si256((__m256i*)(block + 128), _mm256_permute2x128_si256(R4, R5, 0x20));
        _mm256_storeu_si256((__m256i*)(block + 160), _mm256_permute2x128_si256(R6, R7, 0x20));
        
        _mm256_storeu_si256((__m256i*)(block + 192), _mm256_permute2x128_si256(R4, R5, 0x31));
        _mm256_storeu_si256((__m256i*)(block + 224), _mm256_permute2x128_si256(R6, R7, 0x31));

        __m256i in0 = _mm256_loadu_si256((const __m256i*)(in + 0));
        __m256i in1 = _mm256_loadu_si256((const __m256i*)(in + 32));
        __m256i in2 = _mm256_loadu_si256((const __m256i*)(in + 64));
        __m256i in3 = _mm256_loadu_si256((const __m256i*)(in + 96));
        __m256i in4 = _mm256_loadu_si256((const __m256i*)(in + 128));
        __m256i in5 = _mm256_loadu_si256((const __m256i*)(in + 160));
        __m256i in6 = _mm256_loadu_si256((const __m256i*)(in + 192));
        __m256i in7 = _mm256_loadu_si256((const __m256i*)(in + 224));

        _mm256_storeu_si256((__m256i*)(out + 0), _mm256_xor_si256(in0, _mm256_loadu_si256((const __m256i*)(block + 0))));
        _mm256_storeu_si256((__m256i*)(out + 32), _mm256_xor_si256(in1, _mm256_loadu_si256((const __m256i*)(block + 32))));
        _mm256_storeu_si256((__m256i*)(out + 64), _mm256_xor_si256(in2, _mm256_loadu_si256((const __m256i*)(block + 64))));
        _mm256_storeu_si256((__m256i*)(out + 96), _mm256_xor_si256(in3, _mm256_loadu_si256((const __m256i*)(block + 96))));
        _mm256_storeu_si256((__m256i*)(out + 128), _mm256_xor_si256(in4, _mm256_loadu_si256((const __m256i*)(block + 128))));
        _mm256_storeu_si256((__m256i*)(out + 160), _mm256_xor_si256(in5, _mm256_loadu_si256((const __m256i*)(block + 160))));
        _mm256_storeu_si256((__m256i*)(out + 192), _mm256_xor_si256(in6, _mm256_loadu_si256((const __m256i*)(block + 192))));
        _mm256_storeu_si256((__m256i*)(out + 224), _mm256_xor_si256(in7, _mm256_loadu_si256((const __m256i*)(block + 224))));

        m_state[12] += 4;
        in += 256;
        out += 256;
        len -= 256;
    }
    secure_zero(block, sizeof(block));
    
    // Tail blocks
    if (len > 0) {
        encrypt_scalar(in, out, len);
    }
}
#endif

void ChaCha20::encrypt(const uint8_t* in, uint8_t* out, size_t len) {
#if defined(__x86_64__) || defined(_M_X64)
    if (len >= 256) {
        encrypt_avx2(in, out, len);
        return;
    }
#endif
    encrypt_scalar(in, out, len);
}

void ChaCha20::decrypt(const uint8_t* in, uint8_t* out, size_t len) {
    encrypt(in, out, len);
}

} // namespace tinycrypto
