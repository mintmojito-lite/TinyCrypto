#include "curve25519.hpp"
#include "bigint.hpp"
#include "utils.hpp"
#include <cstring>

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

    for (int t = 254; t >= 0; t--) {
        int32_t bit = (clamped[t / 8] >> (t % 8)) & 1;
        swap ^= bit;
        x2.conditionally_swap(x3, swap);
        z2.conditionally_swap(z3, swap);
        swap = bit;

        // Montgomery ladder step
        Fe A = x2 + z2;
        Fe AA = A.square();
        Fe B = x2 - z2;
        Fe BB = B.square();
        Fe E = AA - BB;
        Fe C = x3 + z3;
        Fe D = x3 - z3;
        
        Fe DA = D * A;
        Fe CB = C * B;
        
        x3 = (DA + CB).square();
        z3 = x1 * ((DA - CB).square());
        
        x2 = AA * BB;
        Fe E_times_a24266 = E * Fe(121665, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        z2 = E * (AA + E_times_a24266);
    }
    
    x2.conditionally_swap(x3, swap);
    z2.conditionally_swap(z3, swap);

    // Final mapping back to affine coordinates
    Fe z2_inv = z2.invert();
    Fe res = x2 * z2_inv;
    
    std::array<uint8_t, 32> out;
    res.to_bytes(out.data());
    
    // Scrutinize boundaries using SecureBuffer / secure wipe for clamped secrets
    utils::secure_zero(clamped, 32);
    return out;
}

} // namespace tinycrypto
