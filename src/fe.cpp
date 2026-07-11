#include "bigint.hpp"
#ifdef PICO_ON_DEVICE
#include "pico.h"
#define FORCE_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
#define __not_in_flash(x)
#define FORCE_INLINE __forceinline
#else
#define __not_in_flash(x)
#define FORCE_INLINE __attribute__((always_inline))
#endif

namespace tinycrypto {

// ============================================================================
// 32-bit-only field arithmetic for Cortex-M0+ (no int64_t anywhere)
// ============================================================================

// Carry-normalize: propagate carries so each limb is within its nominal
// 26/25-bit range. Uses only int32_t arithmetic.
static inline void fe_carry_normalize(int32_t h[10]) {
    int32_t c;
    c = h[0] >> 26;  h[1] += c;  h[0] -= c * (1 << 26);
    c = h[1] >> 25;  h[2] += c;  h[1] -= c * (1 << 25);
    c = h[2] >> 26;  h[3] += c;  h[2] -= c * (1 << 26);
    c = h[3] >> 25;  h[4] += c;  h[3] -= c * (1 << 25);
    c = h[4] >> 26;  h[5] += c;  h[4] -= c * (1 << 26);
    c = h[5] >> 25;  h[6] += c;  h[5] -= c * (1 << 25);
    c = h[6] >> 26;  h[7] += c;  h[6] -= c * (1 << 26);
    c = h[7] >> 25;  h[8] += c;  h[7] -= c * (1 << 25);
    c = h[8] >> 26;  h[9] += c;  h[8] -= c * (1 << 26);
    c = h[9] >> 25;  h[0] += c * 19;  h[9] -= c * (1 << 25);
    c = h[0] >> 26;  h[1] += c;  h[0] -= c * (1 << 26);
}

// ---- Multiply helper: 32x32->64 using 16x16 partial products ----
// On Cortex-M0+, each 16x16 multiply is a single MULS instruction.

// Unsigned 32x32 -> 64-bit multiply returning uint64_t.
// Built to avoid __aeabi_lmul.
FORCE_INLINE
static inline uint64_t __not_in_flash("umul32_64") umul32_64(uint32_t a, uint32_t b) {
    uint32_t a_lo = a & 0xFFFFu, a_hi = a >> 16;
    uint32_t b_lo = b & 0xFFFFu, b_hi = b >> 16;
    uint32_t p00 = a_lo * b_lo, p01 = a_lo * b_hi;
    uint32_t p10 = a_hi * b_lo, p11 = a_hi * b_hi;
    uint32_t mid = p01 + p10;
    uint32_t mid_carry = (mid < p01) ? 1u : 0u;
    uint32_t lo = p00 + (mid << 16);
    uint32_t lo_carry = (lo < p00) ? 1u : 0u;
    uint32_t hi = p11 + (mid >> 16) + (mid_carry << 16) + lo_carry;
    return ((uint64_t)hi << 32) | lo;
}

FORCE_INLINE
static inline void __not_in_flash("mul32_acc") mul32_acc(uint32_t& acc_lo, uint32_t& acc_hi,
                             uint32_t a, uint32_t b) {
    // Compute a*b as 64-bit and add to accumulator
    uint32_t a_lo = a & 0xFFFFu, a_hi = a >> 16;
    uint32_t b_lo = b & 0xFFFFu, b_hi = b >> 16;

    uint32_t p00 = a_lo * b_lo;
    uint32_t p01 = a_lo * b_hi;
    uint32_t p10 = a_hi * b_lo;
    uint32_t p11 = a_hi * b_hi;

    // Combine: result = p11:p00 + (p01+p10)<<16
    uint32_t mid = p01 + p10;
    uint32_t mid_carry = (mid < p01) ? 1u : 0u;

    uint32_t prod_lo = p00 + (mid << 16);
    uint32_t prod_lo_carry = (prod_lo < p00) ? 1u : 0u;
    uint32_t prod_hi = p11 + (mid >> 16) + (mid_carry << 16) + prod_lo_carry;

    // Add to accumulator
    uint32_t new_lo = acc_lo + prod_lo;
    uint32_t carry1 = (new_lo < acc_lo) ? 1u : 0u;
    acc_lo = new_lo;
    acc_hi += prod_hi + carry1;
}

Fe Fe::operator+(const Fe& b) const {
    Fe res;
    for (int i = 0; i < 10; i++) res.v[i] = v[i] + b.v[i];
    return res;
}

Fe Fe::operator-(const Fe& b) const {
    Fe res;
    for (int i = 0; i < 10; i++) res.v[i] = v[i] - b.v[i];
    return res;
}

Fe __not_in_flash("operator*") Fe::operator*(const Fe& b) const {
    // Carry-normalize both operands to ensure all limbs are in [0, 2^26) or [0, 2^25).
    int32_t f[10], g[10];
    for (int i = 0; i < 10; i++) { f[i] = v[i]; g[i] = b.v[i]; }
    fe_carry_normalize(f);
    fe_carry_normalize(g);

    // Convert to unsigned and precompute g*19 values.
    uint32_t a[10], bv[10], bg19[10], a2[10];
    for (int i = 0; i < 10; i++) {
        a[i]    = (uint32_t)f[i];
        bv[i]   = (uint32_t)g[i];
        bg19[i] = (uint32_t)(g[i] * 19);
        a2[i]   = (uint32_t)(f[i] * 2);
    }

    // Loop-based schoolbook multiply with modular reduction.
    // For output limb k, sum contributions from all (i,j) where (i+j) mod 10 == k.
    // Wrapped terms (j > 0 when i+j >= 10) use g*19 for reduction mod 2^255-19.
    // On even output limbs, odd-indexed f terms use f*2 (the ×38 = ×2 × ×19 decomposition).
    // On odd output limbs, no doubling is needed.
    uint32_t t_lo[10], t_hi[10];
    for (int k = 0; k < 10; k++) {
        t_lo[k] = 0; t_hi[k] = 0;
        for (int m = 0; m < 10; m++) {
            int i = m;
            int j = (k - m + 10) % 10;
            int wrapped = (i + j >= 10) ? 1 : 0;  // needs reduction
            // On even output limbs, odd-index i terms get ×2 factor
            int use_doubled = (k % 2 == 0) && (i % 2 == 1);

            uint32_t fa = use_doubled ? a2[i] : a[i];
            uint32_t fb = wrapped ? bg19[j] : bv[j];
            mul32_acc(t_lo[k], t_hi[k], fa, fb);
        }
    }

    // Reduce 64-bit accumulators to 26/25-bit limbs via carry propagation.
    Fe res;
    int64_t carry;
    int64_t t64[10];
    for (int i = 0; i < 10; i++) {
        t64[i] = ((int64_t)(uint64_t)t_hi[i] << 32) | t_lo[i];
    }

    carry = t64[0] >> 26;  t64[1] += carry;  t64[0] &= 0x3FFFFFF;
    carry = t64[1] >> 25;  t64[2] += carry;  t64[1] &= 0x1FFFFFF;
    carry = t64[2] >> 26;  t64[3] += carry;  t64[2] &= 0x3FFFFFF;
    carry = t64[3] >> 25;  t64[4] += carry;  t64[3] &= 0x1FFFFFF;
    carry = t64[4] >> 26;  t64[5] += carry;  t64[4] &= 0x3FFFFFF;
    carry = t64[5] >> 25;  t64[6] += carry;  t64[5] &= 0x1FFFFFF;
    carry = t64[6] >> 26;  t64[7] += carry;  t64[6] &= 0x3FFFFFF;
    carry = t64[7] >> 25;  t64[8] += carry;  t64[7] &= 0x1FFFFFF;
    carry = t64[8] >> 26;  t64[9] += carry;  t64[8] &= 0x3FFFFFF;
    carry = t64[9] >> 25;  t64[0] += (int64_t)umul32_64((uint32_t)carry, 19u);  t64[9] &= 0x1FFFFFF;
    carry = t64[0] >> 26;  t64[1] += carry;  t64[0] &= 0x3FFFFFF;

    for (int i = 0; i < 10; i++) res.v[i] = (int32_t)t64[i];
    return res;
}

Fe __not_in_flash("square") Fe::square() const {
    return *this * *this;
}

Fe Fe::invert() const {
    Fe res(1,0,0,0,0,0,0,0,0,0);
    Fe base = *this;
    uint8_t e[32] = { 0xeb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f };
    for (int i = 0; i < 255; i++) {
        uint8_t bit = (e[i / 8] >> (i % 8)) & 1;
        if (bit) res = res * base;
        base = base.square();
    }
    return res;
}

void Fe::conditionally_swap(Fe& other, int32_t iswap) {
    int32_t mask = -iswap;
    for (int i = 0; i < 10; i++) {
        int32_t t = mask & (v[i] ^ other.v[i]);
        v[i] ^= t;
        other.v[i] ^= t;
    }
}

Fe Fe::from_bytes(const uint8_t* b) {
    Fe res;
    BigInt256 n = BigInt256::from_bytes_le(b);
    n.limbs[3] &= 0x7FFFFFFFFFFFFFFF; 
    
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            res.v[i] = n.limbs[0] & 0x3FFFFFF;
            n = n >> 26;
        } else {
            res.v[i] = n.limbs[0] & 0x1FFFFFF;
            n = n >> 25;
        }
    }
    return res;
}

void Fe::to_bytes(uint8_t* b) const {
    int32_t h[10];
    for (int i = 0; i < 10; i++) h[i] = v[i];
    
    fe_carry_normalize(h);
    fe_carry_normalize(h);

    int32_t c;
    c = h[0] >> 26;  h[1] += c;  h[0] -= c * (1 << 26);
    c = h[1] >> 25;  h[2] += c;  h[1] -= c * (1 << 25);
    c = h[2] >> 26;  h[3] += c;  h[2] -= c * (1 << 26);
    c = h[3] >> 25;  h[4] += c;  h[3] -= c * (1 << 25);
    c = h[4] >> 26;  h[5] += c;  h[4] -= c * (1 << 26);
    c = h[5] >> 25;  h[6] += c;  h[5] -= c * (1 << 25);
    c = h[6] >> 26;  h[7] += c;  h[6] -= c * (1 << 26);
    c = h[7] >> 25;  h[8] += c;  h[7] -= c * (1 << 25);
    c = h[8] >> 26;  h[9] += c;  h[8] -= c * (1 << 26);

    BigInt256 res(0,0,0,0);
    int shifts[10] = {0, 26, 51, 77, 102, 128, 153, 179, 204, 230};
    for (int i = 0; i < 10; i++) {
        BigInt256 limb((uint64_t)(uint32_t)h[i], 0, 0, 0);
        res = res + (limb << shifts[i]);
    }
    
    BigInt256 P(0xFFFFFFFFFFFFFFED, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF);
    if (res >= P) {
        res = res - P;
    }
    
    res.to_bytes_le(b);
}

} // namespace tinycrypto
