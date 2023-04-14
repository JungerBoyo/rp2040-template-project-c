/*
    XIP mode linker (code inside flash)
*/

__BOOT_ORIGIN = 0x10000000;
__BOOT_LENGTH = 256;

__FLASH_ORIGIN = 0x10000100;
__FLASH_LENGTH = 2048K - 256;

__SRAM_ORIGIN = 0x20000000;
__SRAM_LENGTH = 264K;


MEMORY
{
    BOOT(rx): ORIGIN = __BOOT_ORIGIN, LENGTH = __BOOT_LENGTH
    FLASH(rx): ORIGIN = __FLASH_ORIGIN, LENGTH = __FLASH_LENGTH
    SRAM(rwx): ORIGIN = __SRAM_ORIGIN, LENGTH = __SRAM_LENGTH
}

SECTIONS
{
    .boot : ALIGN(4) 
    {
        /* keep void startup(void) function (it is not called by user) 
           and --ffunction-sections --gc-sections are on */
        *(.boot*)
        KEEP(*(.boot.startup)) 
        . = 256 - 4;
        LONG(0xCCCCCCCC);
    } > BOOT 

    /* Usual code resides in FLASH (IRQs and ramfuncs reside in ram) */
    .text : ALIGN(4) 
    {
        __text_section_begin = .;
        *(.text*)
        . = ALIGN(4);
        __text_section_end = .;
    } > FLASH

    /* readonly data will be copied into SRAM */
    .rodata : ALIGN(4) 
    {
        __rodata_section_begin = .;
        *(.rodata*)
        . = ALIGN(4);
        __rodata_section_end = .;
    } > SRAM AT> FLASH

    /* contains initialized global data + ramfuncs and IRQs */
    .data : ALIGN(4)
    {
        __data_section_begin = .;
        /* keep IRQ handlers definitions even when they are not called 
           by the user. All IRQs go to ram for performance reasons */
        KEEP(*(.vectors))
        *(.ramfunc .ramfunc.*)
        *(.data*)
        . = ALIGN(4);
        __data_section_end = .;
    } > SRAM AT> FLASH

    .bss : ALIGN(4)
    {
        __bss_section_begin = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_section_end = .;
    } > SRAM

    PROVIDE(__end = .);
    PROVIDE(__flash_begin = ORIGIN(FLASH));
    /* stack grows downwards so set it to the end of SRAM */
    PROVIDE(__stack_top = ORIGIN(SRAM) + LENGTH(SRAM));
}