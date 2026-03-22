#include <gtest/gtest.h>
#include "utils.hpp"
#include "sha256.hpp"
#include <vector>

using namespace tinycrypto::utils;

TEST(UtilsTest, ByteSwap32) {
    EXPECT_EQ(bswap32(0x11223344), 0x44332211);
    EXPECT_EQ(bswap32(0xAABBCCDD), 0xDDCCBBAA);
    EXPECT_EQ(bswap32(0x00000000), 0x00000000);
}

TEST(UtilsTest, ByteSwap64) {
    EXPECT_EQ(bswap64(0x1122334455667788), 0x8877665544332211);
    EXPECT_EQ(bswap64(0x0000000000000000), 0x0000000000000000);
}

TEST(UtilsTest, RotLeft32) {
    EXPECT_EQ(rotl32(0x12345678, 4), 0x23456781);
    EXPECT_EQ(rotl32(0xFF000000, 8), 0x000000FF);
}

TEST(UtilsTest, RotRight32) {
    EXPECT_EQ(rotr32(0x12345678, 4), 0x81234567);
    EXPECT_EQ(rotr32(0x000000FF, 8), 0xFF000000);
}

TEST(UtilsTest, SecureZero) {
    std::vector<uint8_t> buffer(100, 0xFF);
    secure_zero(buffer.data(), buffer.size());
    for (auto val : buffer) {
        EXPECT_EQ(val, 0x00);
    }
}

// SHA-256 Tests
using namespace tinycrypto;

std::string to_hex(const std::array<uint8_t, 32>& hash) {
    char buf[65];
    for (int i = 0; i < 32; ++i) {
        snprintf(buf + i * 2, 3, "%02x", hash[i]);
    }
    return std::string(buf);
}

TEST(SHA256Test, EmptyString) {
    auto hash = SHA256::hash("", 0);
    EXPECT_EQ(to_hex(hash), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST(SHA256Test, AbcString) {
    std::string msg = "abc";
    auto hash = SHA256::hash(msg.data(), msg.length());
    EXPECT_EQ(to_hex(hash), "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

TEST(SHA256Test, Msg448Bits) {
    std::string msg = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    auto hash = SHA256::hash(msg.data(), msg.length());
    EXPECT_EQ(to_hex(hash), "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
}

TEST(SHA256Test, OneMillionA) {
    SHA256 ctx;
    std::string msg(1000, 'a');
    for (int i = 0; i < 1000; ++i) {
        ctx.update(msg.data(), msg.length());
    }
    std::array<uint8_t, 32> hash;
    ctx.finalize(hash.data());
    EXPECT_EQ(to_hex(hash), "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
}
