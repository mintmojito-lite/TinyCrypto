#include <gtest/gtest.h>
#include "hmac.hpp"
#include <string>

using namespace tinycrypto;

namespace {
std::string to_hex(const std::array<uint8_t, 32>& hash) {
    char buf[65];
    for (int i = 0; i < 32; ++i) {
        snprintf(buf + i * 2, 3, "%02x", hash[i]);
    }
    return std::string(buf);
}
}

TEST(HMACTest, RFC4231_Jefe) {
    // Test Case 2 from RFC 4231
    std::string key = "Jefe";
    std::string data = "what do ya want for nothing?";
    
    auto mac = HMAC_SHA256::mac(reinterpret_cast<const uint8_t*>(key.data()), key.length(),
                                reinterpret_cast<const uint8_t*>(data.data()), data.length());
                                
    // Fixed the suffix to correctly match RFC 4231 Test vector standard.
    std::string expected = "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843";
    EXPECT_EQ(to_hex(mac), expected);
}
