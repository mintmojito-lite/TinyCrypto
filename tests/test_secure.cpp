#include <gtest/gtest.h>
#include "utils.hpp"
#include "crypto_primitives.hpp"
#include <vector>
#include <cstring>

using namespace tinycrypto::utils;

TEST(SecureTest, ConstantTimeEqual) {
    std::vector<uint8_t> a = {1, 2, 3, 4, 5};
    std::vector<uint8_t> b = {1, 2, 3, 4, 5};
    std::vector<uint8_t> c = {1, 2, 3, 4, 6};

    EXPECT_TRUE(constant_time_equal(a.data(), b.data(), a.size()));
    EXPECT_FALSE(constant_time_equal(a.data(), c.data(), a.size()));
}

TEST(SecureTest, ExplicitBzeroWipesMemory) {
    std::vector<uint8_t> buf(100, 0xAB);
    
    EXPECT_EQ(buf[0], 0xAB);
    EXPECT_EQ(buf[99], 0xAB);
    
    explicit_bzero(buf.data(), buf.size());

    EXPECT_EQ(buf[0], 0x00);
    EXPECT_EQ(buf[50], 0x00);
    EXPECT_EQ(buf[99], 0x00);
}
