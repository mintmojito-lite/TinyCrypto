#include <gtest/gtest.h>
#include "curve25519.hpp"
#include <string>

using namespace tinycrypto;

namespace {
std::array<uint8_t, 32> hex_to_array(const std::string& hex) {
    std::array<uint8_t, 32> arr;
    for (size_t i = 0; i < 32; ++i) {
        arr[i] = static_cast<uint8_t>(std::stoi(hex.substr(i * 2, 2), nullptr, 16));
    }
    return arr;
}

std::string array_to_hex(const std::array<uint8_t, 32>& arr) {
    char buf[65];
    for (int i = 0; i < 32; ++i) snprintf(buf + i * 2, 3, "%02x", arr[i]);
    return std::string(buf);
}
}

TEST(Curve25519_Test, RFC7748_Vector1) {
    auto scalar = hex_to_array("a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4");
    auto point = hex_to_array("e6db6867583030db3594c1a424b15f7c726624ec26b3353b10a903a6d0ab1c4c");
    auto res = curve25519(scalar, point);
    EXPECT_EQ(array_to_hex(res), "c3da55379de9c6908e94ea4df28d084f32eccf03491c71f754b4075577a28552");
}

TEST(Curve25519_Test, RFC7748_Alice) {
    auto alice_priv = hex_to_array("77076d0a7318a57d3c16c17251b26645df4c2f87ebc0992ab177fba51db92c2a");
    auto base_point = hex_to_array("0900000000000000000000000000000000000000000000000000000000000000"); // Base point X=9
    auto res = curve25519(alice_priv, base_point);
    EXPECT_EQ(array_to_hex(res), "8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a");
}

TEST(Curve25519_Test, RFC7748_DiffieHellman) {
    auto alice_priv = hex_to_array("77076d0a7318a57d3c16c17251b26645df4c2f87ebc0992ab177fba51db92c2a");
    auto bob_priv = hex_to_array("5dab087e624a8a4b79e17f8b83800ee66f3bb1292618b6fd1c2f8b27ff88e0eb");
    
    auto base_point = hex_to_array("0900000000000000000000000000000000000000000000000000000000000000");
    auto alice_pub = curve25519(alice_priv, base_point);
    auto bob_pub = curve25519(bob_priv, base_point);
    
    auto shared1 = curve25519(alice_priv, bob_pub);
    auto shared2 = curve25519(bob_priv, alice_pub);
    
    // Commutativity DH proof
    EXPECT_EQ(array_to_hex(shared1), array_to_hex(shared2));
    
    // Absolute target match proof
    EXPECT_EQ(array_to_hex(shared1), "4a5d9d5ba4ce2de1728e3bf480350f25e07e21c947d19e3376f09b3c1e161742");
}
