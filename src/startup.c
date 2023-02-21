#include <stdint.h>
#include "rp2040.h"

#define FALLBACK __attribute__((weak, alias("irqHandlerDefault")))

FALLBACK void irqHandlerNmi(void);
FALLBACK void irqHandlerHardFault(void);
FALLBACK void irqHandlerSvCall(void);
FALLBACK void irqHandlerPendSv(void);
FALLBACK void irqHandlerSystick(void);
FALLBACK void irqHandlerTimer0(void);
FALLBACK void irqHandlerTimer1(void);
FALLBACK void irqHandlerTimer2(void);
FALLBACK void irqHandlerTimer3(void);
FALLBACK void irqHandlerPwm(void);
FALLBACK void irqHandlerUsbctrl(void);
FALLBACK void irqHandlerXip(void);
FALLBACK void irqHandlerPio00(void);
FALLBACK void irqHandlerPio01(void);
FALLBACK void irqHandlerPio10(void);
FALLBACK void irqHandlerPio11(void);
FALLBACK void irqHandlerDma0(void);
FALLBACK void irqHandlerDma1(void);
FALLBACK void irqHandlerIoBank0(void);
FALLBACK void irqHandlerIoQspi(void);
FALLBACK void irqHandlerSioProc0(void);
FALLBACK void irqHandlerSioProc1(void);
FALLBACK void irqHandlerClocks(void);
FALLBACK void irqHandlerSpi0(void);
FALLBACK void irqHandlerSpi1(void);
FALLBACK void irqHandlerUart0(void);
FALLBACK void irqHandlerUart1(void);
FALLBACK void irqHandlerAdcFifo(void);
FALLBACK void irqHandlerI2c0(void);
FALLBACK void irqHandlerI2c1(void);
FALLBACK void irqHandlerRtc(void);

void __stack_top(void);

__attribute__((used, section(".vectors"))) 
void(*const vectors[])(void) = {
    __stack_top,        // init stack pointer
    0,                  // reset IRQ
    irqHandlerNmi,
    irqHandlerHardFault,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    irqHandlerSvCall,
    0,
    0,
    irqHandlerPendSv,
    irqHandlerSystick,
    irqHandlerTimer0,
    irqHandlerTimer1,
    irqHandlerTimer2,
    irqHandlerTimer3,
    irqHandlerPwm,
    irqHandlerUsbctrl,
    irqHandlerXip,
    irqHandlerPio00,
    irqHandlerPio01,
    irqHandlerPio10,
    irqHandlerPio11,
    irqHandlerDma0,
    irqHandlerDma1,
    irqHandlerIoBank0,
    irqHandlerIoQspi,
    irqHandlerSioProc0,
    irqHandlerSioProc1,
    irqHandlerClocks,
    irqHandlerSpi0,
    irqHandlerSpi1,
    irqHandlerUart0,
    irqHandlerUart1,
    irqHandlerAdcFifo,
    irqHandlerI2c0,
    irqHandlerI2c1,
    irqHandlerRtc
};

extern int main(void);

extern uint32_t __text_section_begin;
extern uint32_t __text_section_end;
extern uint32_t __data_section_begin;
extern uint32_t __data_section_end;
extern uint32_t __bss_section_begin;
extern uint32_t __bss_section_end;

extern uint32_t __flash_begin;

void irqHandlerDefault() { while(1); }

__attribute__((naked, used, noreturn, section(".boot.startup")))
void startup(void) {
    /// ???
    XIP_SSI->SSIENR = 0;

    XIP_SSI->BAUDR = 2;

    XIP_SSI->CTRLR0 =
        (XIP_SSI_CTRLR0_SPI_FRF_STD << XIP_SSI_CTRLR0_SPI_FRF_Pos)   |
        (XIP_SSI_CTRLR0_TMOD_EEPROM_READ << XIP_SSI_CTRLR0_TMOD_Pos) |
        ((32-1) << XIP_SSI_CTRLR0_DFS_32_Pos);

    XIP_SSI->CTRLR1 = (0 << XIP_SSI_CTRLR1_NDF_Pos);

    XIP_SSI->SPI_CTRLR0 = 
        (0x03/*READ_DATA*/ << XIP_SSI_SPI_CTRLR0_XIP_CMD_Pos) |
        ((24 / 4) << XIP_SSI_SPI_CTRLR0_ADDR_L_Pos) |
        (XIP_SSI_SPI_CTRLR0_INST_L_8B << XIP_SSI_SPI_CTRLR0_INST_L_Pos) |
        (XIP_SSI_SPI_CTRLR0_TRANS_TYPE_1C1A << XIP_SSI_SPI_CTRLR0_TRANS_TYPE_Pos);

    XIP_SSI->SSIENR = XIP_SSI_SSIENR_SSI_EN_Msk;

    uint32_t* src = &__flash_begin;
    uint32_t* dst = &__text_section_begin;
    // copy text and data sections from LMD to VMD (from flash to ram)
    while (dst < &__data_section_end) {
        *dst++ = *src++;
    }

    // zero-initialize bss section
    dst = &__bss_section_begin;
    while (dst < &__bss_section_end) {
        *dst++ = 0;
    }

    // set interrupt vector table
    SCB->VTOR = (uint32_t)vectors;

    // branch into main
    main();

    while(1);
}