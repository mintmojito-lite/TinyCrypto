/**
 * bare-metal firmware entry point for ARM Cortex-M7 (QEMU mps2-an500)
 *
 * No OS.  No libc.  UART only.
 * -----------------------------------------------------------------------
 * MPS2-AN500 CMSDK APB UART0 base: 0x40004000
 *   Offset 0x00  UART_DATA   (byte to transmit)
 *   Offset 0x04  UART_STATE  (bit 0 = TX full / TX busy)
 *   Offset 0x08  UART_CTRL   (bit 0 = TX enable)
 * -----------------------------------------------------------------------
 */

#include "sha256.hpp"
#include <cstdint>

/* --------------------------------------------------------------------------
 * Vector table  –  must land at address 0x00000000 (start of FLASH)
 * -------------------------------------------------------------------------- */
extern "C" {
    void reset_handler() __attribute__((noreturn));
    extern uint32_t __stack_top;

    __attribute__((section(".vector_table"), used))
    void* const vector_table[2] = {
        (void*)&__stack_top,
        (void*)reset_handler,
    };
}

/* --------------------------------------------------------------------------
 * CMSDK APB UART0 at 0x40004000
 *   DATA   +0x00  write byte to transmit
 *   STATE  +0x04  bit 0 = TX buffer full
 *   CTRL   +0x08  bit 0 = TX enable, bit 1 = RX enable
 * -------------------------------------------------------------------------- */
struct CmsdkUart {
    volatile uint32_t DATA;   // 0x00
    volatile uint32_t STATE;  // 0x04
    volatile uint32_t CTRL;   // 0x08
    volatile uint32_t INTSTATUS; // 0x0C
    volatile uint32_t BAUDDIV;   // 0x10
};

static CmsdkUart* const UART0 = reinterpret_cast<CmsdkUart*>(0x40004000);

static void uart_init() {
    UART0->BAUDDIV = 16;          // divisor for ~1MHz UART clock / 16 = 62500 baud
    UART0->CTRL    = (1u << 0);   // TX enable
}

static void uart_putc(char c) {
    while (UART0->STATE & 1u) {} // wait while TX full
    UART0->DATA = static_cast<uint32_t>(c);
}

static void uart_puts(const char* s) {
    while (*s) uart_putc(*s++);
}

/* --------------------------------------------------------------------------
 * Hex helpers
 * -------------------------------------------------------------------------- */
static const char HEX[] = "0123456789abcdef";

static void uart_hex_byte(uint8_t b) {
    uart_putc(HEX[b >> 4]);
    uart_putc(HEX[b & 0xF]);
}

/* --------------------------------------------------------------------------
 * .bss / .data init  (normally done by startup code)
 * -------------------------------------------------------------------------- */
extern "C" {
    extern uint32_t __bss_start, __bss_end;
    extern uint32_t __data_start, __data_end, __data_load;
}

static void init_memory() {
    for (uint32_t* p = &__bss_start; p < &__bss_end; ++p) *p = 0;
    uint32_t* src = &__data_load;
    for (uint32_t* dst = &__data_start; dst < &__data_end; ) *dst++ = *src++;
}

/* --------------------------------------------------------------------------
 * Firmware entry point
 * -------------------------------------------------------------------------- */
extern "C" void reset_handler() {
    init_memory();
    uart_init();

    uart_puts("\r\n=== TinyCrypto Bare-Metal Firmware ===\r\n");
    uart_puts("Hashing: \"Hello Edge AI\"\r\n");

    const char msg[] = "Hello Edge AI";
    auto digest = tinycrypto::SHA256::hash(
        reinterpret_cast<const uint8_t*>(msg),
        sizeof(msg) - 1
    );

    uart_puts("SHA-256: ");
    for (uint8_t byte : digest) {
        uart_hex_byte(byte);
    }
    uart_puts("\r\n");

    uart_puts("=== DONE ===\r\n");

    for (;;) {}
}
