#include <gtest/gtest.h>
#include "aes256.hpp"
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>

using namespace tinycrypto;

static std::vector<uint8_t> hex_to_bytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        bytes.push_back(static_cast<uint8_t>(std::stoi(hex.substr(i, 2), nullptr, 16)));
    }
    return bytes;
}

static std::string bytes_to_hex(const uint8_t* bytes, size_t len) {
    std::stringstream ss;
    for(size_t i=0; i<len; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i];
    }
    return ss.str();
}

TEST(AES256Test, NIST_ECB_Encrypt) {
    auto key_bytes = hex_to_bytes("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f");
    std::array<uint8_t, 32> key;
    std::copy(key_bytes.begin(), key_bytes.end(), key.begin());

    auto pt_bytes = hex_to_bytes("00112233445566778899aabbccddeeff");
    
    Aes256Ctx ctx;
    aes256_init(ctx, key);
    
    uint8_t out[16];
    aes256_encrypt_block(ctx, pt_bytes.data(), out);
    
    EXPECT_EQ(bytes_to_hex(out, 16), "8ea2b7ca516745bfeafc49904b496089");
}
