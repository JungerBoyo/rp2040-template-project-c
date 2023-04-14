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

ENTRY(startup)

SECTIONS
{
    .boot : ALIGN(4) 
    {
        /* keep void startup(void) function (it is not called by user) 
           and --ffunction-sections --gc-sections are on */
        KEEP(*(.boot.startup)) 
        *(.boot*)
        . = 256 - 4;
        LONG(0xCCCCCCCC);
    } > BOOT 

    .flash : ALIGN(4)
    {
        *(.flash*)
    } > FLASH

    .text : ALIGN(4) 
    {
        __text_section_begin = .;
        /* keep IRQ handlers definitions even when they are not called 
           by the user */
        KEEP(*(.vectors))
        *(.text*)
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(4);
        __text_section_end = .;
    } > SRAM AT> FLASH

    .data : ALIGN(4)
    {
        __data_section_begin = .;
        *(.ramfunc .ramfunc.*);
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
    PROVIDE(__stack_top = ORIGIN(SRAM) + LENGTH(SRAM));
}