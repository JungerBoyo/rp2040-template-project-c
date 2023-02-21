#include <stdint.h>
#include <rp2040.h>

#define GPIO2 2

int main(void) {
    RESETS->RESET_b.io_bank0 = 0;
    RESETS->RESET_b.pads_bank0 = 0;

    while (
        0 == RESETS->RESET_DONE_b.io_bank0 || 
        0 == RESETS->RESET_DONE_b.pads_bank0
    );

    SIO->GPIO_OE_SET = (1 << GPIO2);
    IO_BANK0->GPIO2_CTRL_b.FUNCSEL = IO_BANK0_GPIO2_CTRL_FUNCSEL_sio_2;
    SIO->GPIO_OUT_CLR = (1 << GPIO2);
    SIO->GPIO_OUT_SET = (1 << GPIO2);

    while(1);
}