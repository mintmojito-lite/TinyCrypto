#pragma once
#include <cstdint>
#include <array>
#include "utils.hpp"

namespace tinycrypto {

class BigInt256 {
public:
    uint64_t limbs[4]; // limbs[0] = least significant

    BigInt256() { }
    BigInt256(uint64_t l0, uint64_t l1, uint64_t l2, uint64_t l3) {
        limbs[0] = l0; limbs[1] = l1; limbs[2] = l2; limbs[3] = l3;
    }

    BigInt256 operator+(const BigInt256& b) const;
    BigInt256 operator*(const BigInt256& b) const;
    BigInt256 operator%(const BigInt256& p) const;
    BigInt256 operator-(const BigInt256& b) const;
    BigInt256 operator<<(int shift) const;
    BigInt256 operator>>(int shift) const;

    BigInt256 mod_inverse(const BigInt256& p) const;
    bool operator==(const BigInt256& b) const;
    bool operator>=(const BigInt256& b) const;

    static BigInt256 from_bytes_le(const uint8_t* bytes);
    void to_bytes_le(uint8_t* bytes) const;
    bool is_zero() const;
};

// 10x 26-bit limbs radix-2^25.5 struct for Curve25519 field mechanics
struct Fe {
    int32_t v[10];

    Fe() { for(int i=0; i<10; i++) v[i] = 0; }
    Fe(int32_t v0, int32_t v1, int32_t v2, int32_t v3, int32_t v4, 
       int32_t v5, int32_t v6, int32_t v7, int32_t v8, int32_t v9) {
        v[0]=v0; v[1]=v1; v[2]=v2; v[3]=v3; v[4]=v4;
        v[5]=v5; v[6]=v6; v[7]=v7; v[8]=v8; v[9]=v9;
    }

    Fe operator+(const Fe& b) const;
    Fe operator-(const Fe& b) const;
    Fe operator*(const Fe& b) const;
    Fe square() const;
    Fe invert() const;
    
    void conditionally_swap(Fe& other, int32_t iswap);
    static Fe from_bytes(const uint8_t* bytes);
    void to_bytes(uint8_t* bytes) const;
};

} // namespace tinycrypto
