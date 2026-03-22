/* Linker script for ARM Cortex-M7 (QEMU mps2-an500)
 * FLASH: 0x00000000, 16MB
 * RAM:   0x20000000, 4MB
 */

ENTRY(reset_handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 16M
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
}

SECTIONS
{
    /* Vector table + code */
    .text :
    {
        KEEP(*(.vector_table))
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
        __text_end = .;
    } > FLASH

    /* Initialized data: lives in FLASH, copied to RAM at boot */
    __data_load = __text_end;
    .data : AT(__data_load)
    {
        __data_start = .;
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    } > RAM

    /* Zero-initialized data */
    .bss :
    {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > RAM

    /* Stack grows downward from top of RAM */
    __stack_top = ORIGIN(RAM) + LENGTH(RAM);
}
