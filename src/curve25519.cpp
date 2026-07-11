#include "curve25519.hpp"
#include "bigint.hpp"
#include "utils.hpp"
#include "crypto_primitives.hpp"
#include <cstring>

// Temporary timing instrumentation - remove after diagnosis
#ifdef PICO_ON_DEVICE
#include "pico/stdlib.h"
#include <cstdio>
#endif

namespace tinycrypto {

std::array<uint8_t, 32> curve25519(const std::array<uint8_t, 32>& scalar, const std::array<uint8_t, 32>& point) {
    uint8_t clamped[32];
    std::memcpy(clamped, scalar.data(), 32);
    
    // X25519 clamping
    clamped[0] &= 248;
    clamped[31] &= 127;
    clamped[31] |= 64;

    Fe x1 = Fe::from_bytes(point.data());
    Fe x2(1,0,0,0,0,0,0,0,0,0);
    Fe z2(0,0,0,0,0,0,0,0,0,0);
    Fe x3 = x1;
    Fe z3(1,0,0,0,0,0,0,0,0,0);

    int32_t swap = 0;

#ifdef PICO_ON_DEVICE
    uint64_t ladder_start = time_us_64();
#endif

    for (int t = 254; t >= 0; t--) {
        int32_t bit = (clamped[t / 8] >> (t % 8)) & 1;
        swap ^= bit;
        x2.conditionally_swap(x3, swap);
        z2.conditionally_swap(z3, swap);
        swap = bit;

        // Montgomery ladder step
        Fe A = x2 + z2;
        Fe B = x2 - z2;
        Fe AA = A.square();
        Fe BB = B.square();
        Fe E = AA - BB;

        // Reuse A as DA, B as CB (A and B are dead after the multiplies)
        {
            Fe C = x3 + z3;
            Fe D = x3 - z3;
            A = D * A;   // A now holds DA
            B = C * B;   // B now holds CB
        } // C, D are dead here

        x3 = (A + B).square();
        z3 = x1 * ((A - B).square());

        x2 = AA * BB;
        z2 = E * (AA + E * Fe(121665, 0, 0, 0, 0, 0, 0, 0, 0, 0));
    }

#ifdef PICO_ON_DEVICE
    uint64_t ladder_end = time_us_64();
#endif
    
    x2.conditionally_swap(x3, swap);
    z2.conditionally_swap(z3, swap);

#ifdef PICO_ON_DEVICE
    uint64_t invert_start = time_us_64();
#endif

    // Final mapping back to affine coordinates
    Fe z2_inv = z2.invert();
    Fe res = x2 * z2_inv;

#ifdef PICO_ON_DEVICE
    uint64_t invert_end = time_us_64();
    printf("[DIAG] Ladder: %llu us  Invert+final_mul: %llu us\n",
           (unsigned long long)(ladder_end - ladder_start),
           (unsigned long long)(invert_end - invert_start));
#endif
    
    std::array<uint8_t, 32> out;
    res.to_bytes(out.data());
    
    // Scrutinize boundaries using SecureBuffer / secure wipe for clamped secrets
    explicit_bzero(clamped, 32);
    return out;
}

} // namespace tinycrypto
