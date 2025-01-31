// test that sw uart works.
#include "rpi.h"
#include "sw-uart.h"

void notmain(void) {
    output("\nTRACE:");
    hw_uart_disable();

    // use pin 14 for tx, 15 for rx
    sw_uart_t u = sw_uart_init(14,15, 115200);

    // print in the most basic way.
    sw_uart_put8(&u, 'h');
    sw_uart_put8(&u, 'e');
    sw_uart_put8(&u, 'l');
    sw_uart_put8(&u, 'l');
    sw_uart_put8(&u, 'o');
    sw_uart_put8(&u, '\n');
    sw_uart_put8(&u, '\n');
    sw_uart_put8(&u, '\n');

    // reset to using the hardware uart.
    uart_init();

    trace("if you see `hello` above, sw uart worked!\n");
}
