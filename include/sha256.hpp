#pragma once

#include <cstdint>
#include <cstddef>
#include <array>

namespace tinycrypto {

class SHA256 {
public:
    static constexpr size_t HASH_SIZE = 32;
    static constexpr size_t BLOCK_SIZE = 64;

    SHA256();

    void update(const void* data, size_t len);
    void finalize(uint8_t hash[HASH_SIZE]);

    // Convenience static method for a single shot hash
    static std::array<uint8_t, HASH_SIZE> hash(const void* data, size_t len);

private:
    void transform();

    uint32_t m_state[8];
    uint8_t m_buffer[BLOCK_SIZE];
    uint64_t m_total_len;
};

} // namespace tinycrypto
