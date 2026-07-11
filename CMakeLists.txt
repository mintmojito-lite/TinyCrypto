cmake_minimum_required(VERSION 3.13)

option(BUILD_HOST_TESTS "Build host-side unit tests" OFF)

if(NOT BUILD_HOST_TESTS)
    if(NOT DEFINED ENV{PICO_SDK_PATH} AND NOT DEFINED PICO_SDK_PATH)
        message(FATAL_ERROR "PICO_SDK_PATH not set.")
    endif()
    if(DEFINED ENV{PICO_SDK_PATH})
        set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
    endif()
    # Pull in Raspberry Pi Pico SDK (must be before project())
    include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)
endif()

project(TinyCrypto C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if(BUILD_HOST_TESTS)
    # ---------------------------------------------------------
    # Host-side unit tests (built with your laptop's compiler, NOT arm-none-eabi)
    # ---------------------------------------------------------
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG        release-1.12.1
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    file(GLOB TINYCRYPTO_TEST_SOURCES "tests/*.cpp")
    file(GLOB TINYCRYPTO_LIB_SOURCES "src/*.cpp")
    list(FILTER TINYCRYPTO_LIB_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

    foreach(test_src ${TINYCRYPTO_TEST_SOURCES})
        get_filename_component(test_name ${test_src} NAME_WE)
        add_executable(${test_name} ${test_src} ${TINYCRYPTO_LIB_SOURCES})
        target_include_directories(${test_name} PRIVATE include)
        target_compile_features(${test_name} PRIVATE cxx_std_17)
        target_link_libraries(${test_name} PRIVATE gtest_main)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
else()
    # Initialize the Raspberry Pi Pico SDK
    pico_sdk_init()

    # Gather all source files
    file(GLOB TINYCRYPTO_SOURCES "src/*.cpp")

    # Define the firmware executable
    add_executable(firmware
        ${TINYCRYPTO_SOURCES}
    )

    # Enforce Cortex-M0+ and memory model constraints
    target_compile_options(firmware PRIVATE
        -mcpu=cortex-m0plus
        -mthumb
        -Os
        -Wall
        -Wextra
        -Werror
        $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    )

    # NOTE: Per-file -O3 override was reverted because the inlined code
    # (operator* grew to 15KB) overflows RP2040's 16KB XIP cache,
    # causing 8x regression. The always_inline attributes in fe.cpp
    # combined with -Os should still force inlining via the attribute.

    target_include_directories(firmware PRIVATE include)

    # Link Pico stdlib (provides minimal startup code without heavy standard libraries)
    target_link_libraries(firmware PRIVATE pico_stdlib)

    # Enable USB stdio and disable UART stdio for serial output over USB
    pico_enable_stdio_usb(firmware 1)
    pico_enable_stdio_uart(firmware 0)

    # Poison dynamic allocation (malloc, calloc, new) using linker wrap options to guarantee stack-only memory model
    target_link_options(firmware PRIVATE
        "-Wl,--wrap=malloc"
        "-Wl,--wrap=calloc"
        "-Wl,--wrap=realloc"
        "-Wl,--wrap=free"
        "-Wl,--wrap=_Znwj" # operator new(unsigned int)
        "-Wl,--wrap=_Znwm" # operator new(unsigned long)
        "-Wl,--wrap=_ZdaPv" # operator delete
        "-Wl,--wrap=_ZdlPv" # operator delete[]
    )

    # Generate the .uf2 binary for dragging and dropping onto the Raspberry Pi Pico
    # pico_add_extra_outputs(firmware)
endif()
