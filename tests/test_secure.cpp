#include <gtest/gtest.h>
#include "utils.hpp"
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

TEST(SecureTest, SecureBufferWipesMemory) {
    uint8_t* ptr = nullptr;
    {
        SecureBuffer<uint8_t> buf(100);
        ptr = buf.data();
        std::memset(ptr, 0xAB, 100);
        
        EXPECT_EQ(ptr[0], 0xAB);
        EXPECT_EQ(ptr[99], 0xAB);
    } 

    // The first 8-16 bytes may be overwritten by the MSVC Release heap free list pointer.
    // However, the rest of the array should remain strictly zeroed out by SecureZeroMemory.
    EXPECT_EQ(ptr[50], 0x00);
    EXPECT_EQ(ptr[99], 0x00);
}
