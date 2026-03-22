#pragma once
#include <array>
#include <cstdint>

namespace tinycrypto {
    // Computes X25519 scalar multiplication: out = scalar * point
    std::array<uint8_t, 32> curve25519(const std::array<uint8_t, 32>& scalar, const std::array<uint8_t, 32>& point);
}
