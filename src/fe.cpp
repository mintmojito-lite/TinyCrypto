#include "bigint.hpp"

namespace tinycrypto {

static inline int64_t ashr(int64_t x, int s) {
    return x >= 0 ? (x >> s) : ~((~x) >> s);
}

static void fe_reduce(int64_t* t, Fe& res) {
    for (int loop = 0; loop < 3; loop++) {
        t[1] += ashr(t[0], 26); t[0] &= 0x3FFFFFF;
        t[2] += ashr(t[1], 25); t[1] &= 0x1FFFFFF;
        t[3] += ashr(t[2], 26); t[2] &= 0x3FFFFFF;
        t[4] += ashr(t[3], 25); t[3] &= 0x1FFFFFF;
        t[5] += ashr(t[4], 26); t[4] &= 0x3FFFFFF;
        t[6] += ashr(t[5], 25); t[5] &= 0x1FFFFFF;
        t[7] += ashr(t[6], 26); t[6] &= 0x3FFFFFF;
        t[8] += ashr(t[7], 25); t[7] &= 0x1FFFFFF;
        t[9] += ashr(t[8], 26); t[8] &= 0x3FFFFFF;
        int64_t c9 = ashr(t[9], 25); t[9] &= 0x1FFFFFF;
        t[0] += c9 * 19;
    }
    for (int i = 0; i < 10; i++) {
        res.v[i] = static_cast<int32_t>(t[i]);
    }
}

Fe Fe::operator+(const Fe& b) const {
    Fe res;
    int64_t t[10];
    for (int i = 0; i < 10; i++) t[i] = (int64_t)v[i] + b.v[i];
    fe_reduce(t, res);
    return res;
}

Fe Fe::operator-(const Fe& b) const {
    Fe res;
    int64_t t[10];
    for (int i = 0; i < 10; i++) t[i] = (int64_t)v[i] - b.v[i];
    fe_reduce(t, res);
    return res;
}

Fe Fe::operator*(const Fe& b) const {
    Fe res;
    int64_t t[10];
    t[0] = v[0]*(int64_t)b.v[0] + v[1]*(int64_t)b.v[9]*38 + v[2]*(int64_t)b.v[8]*19 + v[3]*(int64_t)b.v[7]*38 + v[4]*(int64_t)b.v[6]*19 + v[5]*(int64_t)b.v[5]*38 + v[6]*(int64_t)b.v[4]*19 + v[7]*(int64_t)b.v[3]*38 + v[8]*(int64_t)b.v[2]*19 + v[9]*(int64_t)b.v[1]*38;
    t[1] = v[0]*(int64_t)b.v[1] + v[1]*(int64_t)b.v[0] + v[2]*(int64_t)b.v[9]*19 + v[3]*(int64_t)b.v[8]*19 + v[4]*(int64_t)b.v[7]*19 + v[5]*(int64_t)b.v[6]*19 + v[6]*(int64_t)b.v[5]*19 + v[7]*(int64_t)b.v[4]*19 + v[8]*(int64_t)b.v[3]*19 + v[9]*(int64_t)b.v[2]*19;
    t[2] = v[0]*(int64_t)b.v[2] + v[1]*(int64_t)b.v[1]*2 + v[2]*(int64_t)b.v[0] + v[3]*(int64_t)b.v[9]*38 + v[4]*(int64_t)b.v[8]*19 + v[5]*(int64_t)b.v[7]*38 + v[6]*(int64_t)b.v[6]*19 + v[7]*(int64_t)b.v[5]*38 + v[8]*(int64_t)b.v[4]*19 + v[9]*(int64_t)b.v[3]*38;
    t[3] = v[0]*(int64_t)b.v[3] + v[1]*(int64_t)b.v[2] + v[2]*(int64_t)b.v[1] + v[3]*(int64_t)b.v[0] + v[4]*(int64_t)b.v[9]*19 + v[5]*(int64_t)b.v[8]*19 + v[6]*(int64_t)b.v[7]*19 + v[7]*(int64_t)b.v[6]*19 + v[8]*(int64_t)b.v[5]*19 + v[9]*(int64_t)b.v[4]*19;
    t[4] = v[0]*(int64_t)b.v[4] + v[1]*(int64_t)b.v[3]*2 + v[2]*(int64_t)b.v[2] + v[3]*(int64_t)b.v[1]*2 + v[4]*(int64_t)b.v[0] + v[5]*(int64_t)b.v[9]*38 + v[6]*(int64_t)b.v[8]*19 + v[7]*(int64_t)b.v[7]*38 + v[8]*(int64_t)b.v[6]*19 + v[9]*(int64_t)b.v[5]*38;
    t[5] = v[0]*(int64_t)b.v[5] + v[1]*(int64_t)b.v[4] + v[2]*(int64_t)b.v[3] + v[3]*(int64_t)b.v[2] + v[4]*(int64_t)b.v[1] + v[5]*(int64_t)b.v[0] + v[6]*(int64_t)b.v[9]*19 + v[7]*(int64_t)b.v[8]*19 + v[8]*(int64_t)b.v[7]*19 + v[9]*(int64_t)b.v[6]*19;
    t[6] = v[0]*(int64_t)b.v[6] + v[1]*(int64_t)b.v[5]*2 + v[2]*(int64_t)b.v[4] + v[3]*(int64_t)b.v[3]*2 + v[4]*(int64_t)b.v[2] + v[5]*(int64_t)b.v[1]*2 + v[6]*(int64_t)b.v[0] + v[7]*(int64_t)b.v[9]*38 + v[8]*(int64_t)b.v[8]*19 + v[9]*(int64_t)b.v[7]*38;
    t[7] = v[0]*(int64_t)b.v[7] + v[1]*(int64_t)b.v[6] + v[2]*(int64_t)b.v[5] + v[3]*(int64_t)b.v[4] + v[4]*(int64_t)b.v[3] + v[5]*(int64_t)b.v[2] + v[6]*(int64_t)b.v[1] + v[7]*(int64_t)b.v[0] + v[8]*(int64_t)b.v[9]*19 + v[9]*(int64_t)b.v[8]*19;
    t[8] = v[0]*(int64_t)b.v[8] + v[1]*(int64_t)b.v[7]*2 + v[2]*(int64_t)b.v[6] + v[3]*(int64_t)b.v[5]*2 + v[4]*(int64_t)b.v[4] + v[5]*(int64_t)b.v[3]*2 + v[6]*(int64_t)b.v[2] + v[7]*(int64_t)b.v[1]*2 + v[8]*(int64_t)b.v[0] + v[9]*(int64_t)b.v[9]*38;
    t[9] = v[0]*(int64_t)b.v[9] + v[1]*(int64_t)b.v[8] + v[2]*(int64_t)b.v[7] + v[3]*(int64_t)b.v[6] + v[4]*(int64_t)b.v[5] + v[5]*(int64_t)b.v[4] + v[6]*(int64_t)b.v[3] + v[7]*(int64_t)b.v[2] + v[8]*(int64_t)b.v[1] + v[9]*(int64_t)b.v[0];
    fe_reduce(t, res);
    return res;
}

Fe Fe::square() const {
    return (*this) * (*this);
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
    Fe t = *this;
    int64_t tarr[10];
    for (int i = 0; i < 10; i++) tarr[i] = t.v[i];
    fe_reduce(tarr, t);
    
    int64_t f[10];
    for (int i = 0; i < 10; i++) f[i] = t.v[i];
    
    // Smooth out final ripples into absolute precision bounds to avoid massive BigInt conversion sign-truncation overflow
    f[1] += ashr(f[0], 26); f[0] &= 0x3FFFFFF;
    f[2] += ashr(f[1], 25); f[1] &= 0x1FFFFFF;
    f[3] += ashr(f[2], 26); f[2] &= 0x3FFFFFF;
    f[4] += ashr(f[3], 25); f[3] &= 0x1FFFFFF;
    f[5] += ashr(f[4], 26); f[4] &= 0x3FFFFFF;
    f[6] += ashr(f[5], 25); f[5] &= 0x1FFFFFF;
    f[7] += ashr(f[6], 26); f[6] &= 0x3FFFFFF;
    f[8] += ashr(f[7], 25); f[7] &= 0x1FFFFFF;
    f[9] += ashr(f[8], 26); f[8] &= 0x3FFFFFF;

    BigInt256 res(0,0,0,0);
    int shifts[10] = {0, 26, 51, 77, 102, 128, 153, 179, 204, 230};
    for (int i = 0; i < 10; i++) {
        BigInt256 limb((uint64_t)f[i], 0, 0, 0);
        res = res + (limb << shifts[i]);
    }
    
    BigInt256 P(0xFFFFFFFFFFFFFFED, 0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF, 0x7FFFFFFFFFFFFFFF);
    if (res >= P) {
        res = res - P;
    }
    
    res.to_bytes_le(b);
}

} // namespace tinycrypto
