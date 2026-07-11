#pragma once

#include <cstdint>
#include <cstddef>

#ifdef _MSC_VER
#include <stdlib.h>
#endif

namespace tinycrypto {
namespace utils {

inline uint32_t bswap32(uint32_t x) {
#if defined(_MSC_VER)
    return _byteswap_ulong(x);
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap32(x);
#else
    return ((x & 0xFF000000u) >> 24) |
           ((x & 0x00FF0000u) >>  8) |
           ((x & 0x0000FF00u) <<  8) |
           ((x & 0x000000FFu) << 24);
#endif
}

inline uint64_t bswap64(uint64_t x) {
#if defined(_MSC_VER)
    return _byteswap_uint64(x);
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_bswap64(x);
#else
    return ((x & 0xFF00000000000000ull) >> 56) |
           ((x & 0x00FF000000000000ull) >> 40) |
           ((x & 0x0000FF0000000000ull) >> 24) |
           ((x & 0x000000FF00000000ull) >>  8) |
           ((x & 0x00000000FF000000ull) <<  8) |
           ((x & 0x0000000000FF0000ull) << 24) |
           ((x & 0x000000000000FF00ull) << 40) |
           ((x & 0x00000000000000FFull) << 56);
#endif
}

inline uint32_t rotl32(uint32_t x, int n) {
#if defined(_MSC_VER)
    return _rotl(x, n);
#else
    return (x << n) | (x >> (32 - n));
#endif
}

inline uint32_t rotr32(uint32_t x, int n) {
#if defined(_MSC_VER)
    return _rotr(x, n);
#else
    return (x >> n) | (x << (32 - n));
#endif
}



// Compare two byte arrays in constant time to prevent timing attacks
inline bool constant_time_equal(const uint8_t* a, const uint8_t* b, std::size_t len) {
    uint8_t diff = 0;
    for (std::size_t i = 0; i < len; i++) {
        diff |= (a[i] ^ b[i]);
    }
    return diff == 0;
}



} // namespace utils
} // namespace tinycrypto
