#include <stdint.h>
#include <stddef.h>
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

extern int main(void);

extern uint32_t __text_section_begin;
extern uint32_t __text_section_end;
extern uint32_t __data_section_begin;
extern uint32_t __data_section_end;
extern uint32_t __bss_section_begin;
extern uint32_t __bss_section_end;

#ifndef COPY_CODE_TO_SRAM

extern uint32_t __rodata_section_begin;
extern uint32_t __rodata_section_end;

#endif

extern uint32_t __flash_begin;

void __stack_top(void);

__attribute__((used, section(".text"))) 
// __attribute__((used, section(".flash"))) 
void irqHandlerReset(void) {
// #ifdef COPY_CODE_TO_SRAM
// #ifndef SPI_CONF_GENERIC
//     uint32_t* src = &__flash_begin;
//     uint32_t* dst = &__text_section_begin;
//     // copy text and data sections from LMD to VMD (from flash to sram)
//     while (dst < &__data_section_end) {
//         *dst++ = *src++;
//     }

//     while (XIP_SSI->SR_b.BUSY); // wait for SSI transfer to end
//     XIP_SSI->SSIENR = 0; // disable SSI after copying code to SRAM
// #else 
//     uint32_t* dst = &__data_section_end;
// #endif
// #else
//     // uint32_t* src = (uint32_t*)((uint32_t)(&__flash_begin) + (uint32_t)(&__text_section_end));
//     // uint32_t* dst = &__rodata_section_begin;
//     // // copy rodata and data sections from LMD to VMD (from flash to sram)
//     // while (dst < &__data_section_end) {
//     //     *dst++ = *src++;
//     // }
// #endif

// #ifdef COPY_CODE_TO_SRAM
//     uint32_t* src = &__flash_begin;
//     uint32_t* dst = &__text_section_begin;
//     // copy text and data sections from LMD to VMD (from flash to sram)
//     while (dst < &__data_section_end) {
//         *dst++ = *src++;
//     }

//     while (XIP_SSI->SR_b.BUSY); // wait for SSI transfer to end
//     XIP_SSI->SSIENR = 0; // disable SSI after copying code to SRAM
// #else 
//     uint32_t* dst = &__data_section_end;
// #endif

    uint32_t* dst = &__data_section_end;
    // zero-initialize bss section
    dst = &__bss_section_begin;
    while (dst < &__bss_section_end) {
        *dst++ = 0;
    }
    // branch into main
    main();
}

__attribute__((used, section(".vectors"))) 
void(*const vectors[])(void) = {
    __stack_top,        // init stack pointer
    irqHandlerReset,    // reset IRQ
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

void irqHandlerDefault() { 
    while(1); 
}

__attribute__((naked, section(".boot.waitSSI")))
void waitSSI() {
    asm(".thumb            \n"
        "    .syntax unified            \n"
        "    push {r0, r1, lr}          \n"
        "1:                             \n"
        "    ldr  r1, =%[XIP_SSI_SR]     \n"
        "    ldr  r0, [r1]               \n"
        "    movs r1, %[TFE_Msk]         \n"
        "    tst  r0, r1                 \n"
        "    beq  1b                     \n"
        "    movs r1, %[BUSY_Msk]        \n"
        "    tst  r0, r1                 \n"
        "    bne  1b                     \n"
        "    pop  {r0, r1, pc}"
        :: 
        [XIP_SSI_SR] "i" (&XIP_SSI->SR),
        [TFE_Msk]    "i" (XIP_SSI_SR_TFE_Msk),
        [BUSY_Msk]   "i" (XIP_SSI_SR_BUSY_Msk)
    );
}

__attribute__((naked, used, noreturn, section(".boot.startup")))
void startup(void) {
#ifdef SPI_CONF_GENERIC
    /* must disable SSI for the time of configuration */
    XIP_SSI->SSIENR = 0;

    /* baud rate div 2 minimum */
    XIP_SSI->BAUDR = 4;
    
    XIP_SSI->CTRLR0 = (
        XIP_SSI_CTRLR0_SPI_FRF_STD <<  XIP_SSI_CTRLR0_SPI_FRF_Pos | // STD mode
        XIP_SSI_CTRLR0_TMOD_EEPROM_READ << XIP_SSI_CTRLR0_TMOD_Pos | // eeprom read
        31 << XIP_SSI_CTRLR0_DFS_32_Pos // max 32 bits data frame
    );
    
    XIP_SSI->CTRLR1_b.NDF = 0; // single read

    XIP_SSI->SPI_CTRLR0 = (
        0x03 << XIP_SSI_SPI_CTRLR0_XIP_CMD_Pos | // read cmd
        6 << XIP_SSI_SPI_CTRLR0_ADDR_L_Pos | // 24 bits (24 address)    
        XIP_SSI_SPI_CTRLR0_INST_L_8B << XIP_SSI_SPI_CTRLR0_INST_L_Pos | // 8b instruction (0x03)
        XIP_SSI_SPI_CTRLR0_TRANS_TYPE_1C1A <<  XIP_SSI_SPI_CTRLR0_TRANS_TYPE_Pos // cmd in std, address in std 
    );
    
    /* enable SSI */
    XIP_SSI->SSIENR = 1;
#else
    asm(".thumb            \n"
        "    .syntax unified                    \n"
        "    ldr  r0, =%[_PADS_QSPI]            \n"
        "    movs r1, %[QSPI_SCLK_CONF]         \n"
        "    str  r1, [r0, %[QSPI_SCLK_OFFSET]] \n"
        "    ldr  r2, [r0, %[QSPI_SD0_OFFSET]]  \n"
        "    movs r1, %[QSPI_SDX_SCHMITT_Msk]   \n"
        "    bics r1, r2                        \n"
        "    str  r1, [r0, %[QSPI_SD0_OFFSET]]  \n"
        "    str  r1, [r0, %[QSPI_SD1_OFFSET]]  \n"
        "    str  r1, [r0, %[QSPI_SD2_OFFSET]]  \n"
        "    str  r1, [r0, %[QSPI_SD3_OFFSET]]  \n"
        ::
        [_PADS_QSPI] "i" (PADS_QSPI_BASE),
        [QSPI_SCLK_CONF] "i" (
            PADS_QSPI_GPIO_QSPI_SCLK_DRIVE_8mA << PADS_QSPI_GPIO_QSPI_SCLK_DRIVE_Pos |
            1 << PADS_QSPI_GPIO_QSPI_SCLK_SLEWFAST_Pos
        ),
        [QSPI_SCLK_OFFSET] "i" (offsetof(PADS_QSPI_Type, GPIO_QSPI_SCLK)),
        [QSPI_SDX_SCHMITT_Msk] "i" (PADS_QSPI_GPIO_QSPI_SD0_SCHMITT_Msk),
        [QSPI_SD0_OFFSET] "i" (offsetof(PADS_QSPI_Type, GPIO_QSPI_SD0)),
        [QSPI_SD1_OFFSET] "i" (offsetof(PADS_QSPI_Type, GPIO_QSPI_SD1)),
        [QSPI_SD2_OFFSET] "i" (offsetof(PADS_QSPI_Type, GPIO_QSPI_SD2)),
        [QSPI_SD3_OFFSET] "i" (offsetof(PADS_QSPI_Type, GPIO_QSPI_SD3))
    );
     
    // PADS_QSPI->GPIO_QSPI_SCLK = (
    //     PADS_QSPI_GPIO_QSPI_SCLK_DRIVE_8mA << PADS_QSPI_GPIO_QSPI_SCLK_DRIVE_Pos |
    //     1 << PADS_QSPI_GPIO_QSPI_SCLK_SLEWFAST_Pos
    // );
    // // disable schmitt trigger to reduce delay
    // PADS_QSPI->GPIO_QSPI_SD0 &= ~(PADS_QSPI_GPIO_QSPI_SD0_SCHMITT_Msk);
    // PADS_QSPI->GPIO_QSPI_SD1 &= ~(PADS_QSPI_GPIO_QSPI_SD1_SCHMITT_Msk);
    // PADS_QSPI->GPIO_QSPI_SD2 &= ~(PADS_QSPI_GPIO_QSPI_SD2_SCHMITT_Msk);
    // PADS_QSPI->GPIO_QSPI_SD3 &= ~(PADS_QSPI_GPIO_QSPI_SD3_SCHMITT_Msk);

    ///////////////////////////////////////////////////////////////////////
    // To enable QSPI, perform write to SR2 (status register 2) setting  //
    // second bit (QE) to 1. If bit already set skip (go to QSPI config) //
    ///////////////////////////////////////////////////////////////////////

    // configure as TXRX
    XIP_SSI->SSIENR = 0; // must disable SSI for the time of configuration
    XIP_SSI->BAUDR = 4; // baud rate div 2 minimum
    // XIP_SSI->RX_SAMPLE_DLY = 1; // one cycle sample delay
    XIP_SSI->CTRLR0 = (
        7 << XIP_SSI_CTRLR0_DFS_32_Pos |  // ssi_data_frame_size 8 bits per frame
        XIP_SSI_CTRLR0_TMOD_TX_AND_RX << XIP_SSI_CTRLR0_TMOD_Pos // ssi_transfer_mode 
    );
    XIP_SSI->SSIENR = 1;

    #define CMD_READ_SR1     0x05 // W25Q16JV docs 9.2.4
    #define CMD_READ_SR2     0x35 // W25Q16JV docs 9.2.4
    #define CMD_WRITE_SR2    0x01 // W25Q16JV docs 9.2.5
    #define CMD_WRITE_ENABLE 0x06 // W25Q16JV docs 9.2.1
    #define SR2_QE_Msk       0x02 // W25Q16JV docs 8
    #define SR1_BUSY_Msk     0x01 // W25Q16JV docs 7.1

    // first read value of SR2
    XIP_SSI->DR0 = CMD_READ_SR2;
    XIP_SSI->DR0 = CMD_READ_SR2; // dummy
    waitSSI(); // wait for SSI to complete
    (void)XIP_SSI->DR0; //discard first byte

    /////////////////////
    // Set QE bit to 1 //
    /////////////////////
    if (XIP_SSI->DR0 != SR2_QE_Msk) {
        XIP_SSI->DR0 = CMD_WRITE_ENABLE; // enable writing to SR
        waitSSI(); // wait for SSI to complete
        (void)XIP_SSI->DR0; // discard received value

        // write to SR
        XIP_SSI->DR0 = CMD_WRITE_SR2;
        XIP_SSI->DR0 = 0;
        XIP_SSI->DR0 = SR2_QE_Msk;
        waitSSI(); // wait for SSI to complete
        
        (void)XIP_SSI->DR0; //discard byte
        (void)XIP_SSI->DR0; //discard byte
        (void)XIP_SSI->DR0; //discard byte

        do {
            XIP_SSI->DR0 = CMD_READ_SR1;
            XIP_SSI->DR0 = CMD_READ_SR1; // dummy
            waitSSI(); // wait for SSI to complete
            (void)XIP_SSI->DR0; //discard first byte

        } while ((XIP_SSI->DR0 & SR1_BUSY_Msk) > 0); // wait for QE bit to be set
    }

    ////////////////////////////////////////////////
    // configure ssi in quad SPI mode + fast read //
    ////////////////////////////////////////////////    
    #define CMD_QUAD_FAST_READ 0xEB             // W25Q16JV docs 9.2.11
    #define CMD_QUAD_FAST_READ_CONT_BITS 0xA0   // W25Q16JV docs 9.2.11

    XIP_SSI->SSIENR = 0; // must disable SSI for the time of configuration
    XIP_SSI->CTRLR0 = (
        XIP_SSI_CTRLR0_SPI_FRF_QUAD <<  XIP_SSI_CTRLR0_SPI_FRF_Pos | // QUAD mode
        XIP_SSI_CTRLR0_TMOD_EEPROM_READ << XIP_SSI_CTRLR0_TMOD_Pos | // eeprom read
        31 << XIP_SSI_CTRLR0_DFS_32_Pos // max 32 bits data frame
    );
    XIP_SSI->CTRLR1_b.NDF = 0; // single read
    XIP_SSI->SPI_CTRLR0 = (
        8 << XIP_SSI_SPI_CTRLR0_ADDR_L_Pos | // 32 bits (24 address + 8 mode)    
        4 << XIP_SSI_SPI_CTRLR0_WAIT_CYCLES_Pos | // 4 wait cycles W25Q16JV docs 9.2.11
        XIP_SSI_SPI_CTRLR0_INST_L_8B << XIP_SSI_SPI_CTRLR0_INST_L_Pos | // 8 bit instruction
        XIP_SSI_SPI_CTRLR0_TRANS_TYPE_1C2A << XIP_SSI_SPI_CTRLR0_TRANS_TYPE_Pos // cmd in std, address in quad
    );
    XIP_SSI->SSIENR = 1;

    XIP_SSI->DR0 = CMD_QUAD_FAST_READ;
    XIP_SSI->DR0 = CMD_QUAD_FAST_READ_CONT_BITS; 
    //(next 24 address are 0)
    waitSSI();

    XIP_SSI->SSIENR = 0;

    ////////////////////////////////////////////////////////////
    // reconfigure ssi, now data is sent in format CMD + ADDR //
    // to enable bus access translation set spi to sent data  //
    // in format ADDR + CMD. Where CMD is CONT_BITS           //
    ////////////////////////////////////////////////////////////    
    XIP_SSI->SPI_CTRLR0 = (
        CMD_QUAD_FAST_READ_CONT_BITS << XIP_SSI_SPI_CTRLR0_XIP_CMD_Pos | // cont bits for fast quad spi XIP
        8 << XIP_SSI_SPI_CTRLR0_ADDR_L_Pos | // 32 bits (24 address + 8 mode)    
        4 << XIP_SSI_SPI_CTRLR0_WAIT_CYCLES_Pos | // 4 wait cycles W25Q16JV docs 9.2.11
        XIP_SSI_SPI_CTRLR0_INST_L_NONE << XIP_SSI_SPI_CTRLR0_INST_L_Pos | // no cmd prepending address
        XIP_SSI_SPI_CTRLR0_TRANS_TYPE_2C2A <<  XIP_SSI_SPI_CTRLR0_TRANS_TYPE_Pos // cmd in quad, address in quad
    );

    XIP_SSI->SSIENR = 1;
#endif

// code resides in text section, in order to call reset handler 
// copy text and data to sram
#ifdef COPY_CODE_TO_SRAM
// for generic conf XIP doesn't work so perform code copy to sram in boot2
#ifdef SPI_CONF_GENERIC 
    uint32_t* src = &__flash_begin;
    uint32_t* dst = &__text_section_begin;
    // copy text and data sections from LMD to VMD (from flash to sram)
    while (dst < &__data_section_end) {
        *dst++ = *src++;
    }

    while (XIP_SSI->SR_b.BUSY); // wait for SSI transfer to end
    XIP_SSI->SSIENR = 0; // disable SSI after copying code to SRAM
#else 

#endif
#endif

    // set interrupt vector table
    SCB->VTOR = (uint32_t)vectors;

    irqHandlerReset();
    
    __builtin_unreachable();
}