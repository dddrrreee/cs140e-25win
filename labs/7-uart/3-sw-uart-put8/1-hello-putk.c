// test that sw uart works.
#include "rpi.h"
#include "sw-uart.h"

void notmain(void) {
    output("\nTRACE:");

    hw_uart_disable();

    // use pin 14 for tx, 15 for rx
    sw_uart_t u = sw_uart_init(14,15, 115200);


    sw_uart_putk(&u, "hello!\n\n");

    // reset to using the hardware uart.
    uart_init();

    trace("if you see `hello` above, sw uart worked!\n");
}
