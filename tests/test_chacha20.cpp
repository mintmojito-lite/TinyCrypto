#include <gtest/gtest.h>
#include "chacha20.hpp"
#include <vector>
#include <string>

using namespace tinycrypto;

std::string to_hex(const uint8_t* data, size_t len) {
    char buf[3];
    std::string res;
    res.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        snprintf(buf, sizeof(buf), "%02x", data[i]);
        res += buf;
    }
    return res;
}

std::vector<uint8_t> from_hex(const std::string& hex) {
    std::vector<uint8_t> res;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        uint8_t byte = (uint8_t)strtol(byteString.c_str(), NULL, 16);
        res.push_back(byte);
    }
    return res;
}

TEST(ChaCha20Test, RFC7539_Keystream) {
    uint8_t key[32] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f
    };
    uint8_t nonce[12] = {
        0x00,0x00,0x00,0x09,0x00,0x00,0x00,0x4a,0x00,0x00,0x00,0x00
    };
    uint32_t counter = 1;

    ChaCha20 ctx(key, nonce, counter);

    std::vector<uint8_t> plaintext(64, 0); // XOR 0 gets keystream directly
    std::vector<uint8_t> ciphertext(64, 0);

    ctx.encrypt(plaintext.data(), ciphertext.data(), 64);

    std::string expected = 
        "10f1e7e4d13b5915500fdd1fa32071c4"
        "c7d1f4c733c068030422aa9ac3d46c4e"
        "d2826446079faa0914c2d705d98b02a2"
        "b5129cd1de164eb9cbd083e8a2503c4e";
        
    EXPECT_EQ(to_hex(ciphertext.data(), 64), expected);
}

TEST(ChaCha20Test, RFC7539_Sunscreen) {
    std::string plaintext_str = "Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.";
    std::vector<uint8_t> plaintext(plaintext_str.begin(), plaintext_str.end());

    uint8_t key[32] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
        0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f
    };
    // RFC 7539 Section 2.4.2 Sunscreen test vector specifies nonce 000000000000004a00000000
    uint8_t nonce[12] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4a,0x00,0x00,0x00,0x00 
    };
    uint32_t counter = 1;

    ChaCha20 ctx(key, nonce, counter);
    std::vector<uint8_t> ciphertext(plaintext.size(), 0);

    ctx.encrypt(plaintext.data(), ciphertext.data(), plaintext.size());

    std::string expected_hex = 
        "6e2e359a2568f98041ba0728dd0d6981"
        "e97e7aec1d4360c20a27afccfd9fae0b"
        "f91b65c5524733ab8f593dabcd62b357"
        "1639d624e65152ab8f530c359f0861d8"
        "07ca0dbf500d6a6156a38e088a22b65e"
        "52bc514d16ccf806818ce91ab7793736"
        "5af90bbf74a35be6b40b8eedf2785e42"
        "874d";

    EXPECT_EQ(to_hex(ciphertext.data(), ciphertext.size()), expected_hex);
}
