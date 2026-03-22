#include "utils.hpp"

#include <cstring>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <string.h>
#elif defined(__linux__) || defined(__unix__)
#include <string.h>
#endif

namespace tinycrypto {
namespace utils {

void secure_zero(void* ptr, std::size_t len) {
    if (!ptr || len == 0) return;

#if defined(_WIN32)
    SecureZeroMemory(ptr, len);
#else
    #if defined(__STDC_LIB_EXT1__)
        memset_s(ptr, len, 0, len);
    #elif defined(__GNUC__) || defined(__clang__)
        memset(ptr, 0, len);
        __asm__ __volatile__("" : : "r"(ptr) : "memory");
    #else
        volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
        while (len--) {
            *p++ = 0;
        }
    #endif
#endif
}

} // namespace utils
} // namespace tinycrypto
