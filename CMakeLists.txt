cmake_minimum_required(VERSION 3.14)
project(TinyCrypto VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/O2 /arch:AVX2 /W4 /GS /DYNAMICBASE /NXCOMPAT /guard:cf)
    add_link_options(/DYNAMICBASE /NXCOMPAT /guard:cf)
else()
    add_compile_options(-O3 -Wall -Wextra -Wformat -Wformat-security)
    if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|ARM)")
        add_compile_options(-mavx2 -fstack-protector-strong)
    endif()
    add_compile_definitions(_FORTIFY_SOURCE=2)
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Generic")
include(FetchContent)

# Fetch GTest
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Fetch Benchmark
FetchContent_Declare(
  googlebenchmark
  URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)
endif()

include_directories(include)

# Main Library
set(TINYCRYPTO_SOURCES
    src/sha256.cpp
    src/utils.cpp
    src/chacha20.cpp
    src/hmac.cpp
    src/bigint.cpp
    src/fe.cpp
    src/curve25519.cpp
    src/aes256.cpp
)
# We will use objective creation, ensuring static compilation
if(TINYCRYPTO_SOURCES)
    add_library(tinycrypto STATIC ${TINYCRYPTO_SOURCES})
else()
    # Dummy to allow CMake config before source files exist
    add_library(tinycrypto INTERFACE)
endif()
target_include_directories(tinycrypto PUBLIC include)

# Bare-metal firmware (ARM only)
if(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    add_executable(firmware.elf src/main.cpp)
    target_link_libraries(firmware.elf PRIVATE tinycrypto)
    target_include_directories(firmware.elf PRIVATE include)
    target_link_options(firmware.elf PRIVATE
        -T${CMAKE_CURRENT_SOURCE_DIR}/linker.ld
        -nostartfiles
        --specs=nano.specs
        -Wl,--gc-sections
        -Wl,-Map=${CMAKE_BINARY_DIR}/firmware.map
    )
endif()

# Tests
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Generic")
if(TRUE) # Always create test executable with explicit sources
    add_executable(tinycrypto_tests
    tests/test_sha256.cpp
    tests/test_chacha20.cpp
    tests/test_secure.cpp
    tests/test_hmac.cpp
    tests/test_curve25519.cpp
    tests/test_aes256.cpp
)
    target_link_libraries(tinycrypto_tests PRIVATE tinycrypto gtest_main)
    include(GoogleTest)
    gtest_discover_tests(tinycrypto_tests)
endif()

# Benchmarks
file(GLOB BENCH_SOURCES "benchmarks/*.cpp")
if(BENCH_SOURCES)
    add_executable(tinycrypto_bench ${BENCH_SOURCES})
    target_link_libraries(tinycrypto_bench PRIVATE tinycrypto benchmark::benchmark_main)
endif()
endif()
