// test software uart.
//   1. that it works at all.
//   2. that we can override the system putc with it.
#include "rpi.h"
#include "sw-uart.h"

static sw_uart_t sys_uart;
static unsigned putc_cnt=0;
static rpi_putchar_t old_putc;

int sw_uart_putc(int chr) {
    sw_uart_put8(&sys_uart,chr);
    putc_cnt++;
    return chr;
}

// install sw uart as global system uart.
void sw_uart_set_putchar(void) {
    // 0. flush out all the uart output.  [Q: if not?]
    uart_flush_tx();

    // 1. have to reset the sw uart.  Q: if not?
    sys_uart = sw_uart_init(14,15, 115200);

    // 2. override system putc.
    old_putc = rpi_putchar_set(sw_uart_putc);

    putc_cnt=0;
}

// reset the hardware uart and install the old putc.
void hw_uart_set_putchar(void) {
    // Q: what happens if we don't do this?
    uart_init();    
    rpi_putchar_set(old_putc);
}

void notmain(void) {
    /*************************************************************
     * test 1: see that uart works at all.
     */
    trace("test 1: about to print `hello world` 10x\n");
    trace("if your pi locks up, it means you are not transmitting\n");

    // turn off the hw UART so can use the same device.
    hw_uart_disable();

    // use pin 14 for tx, 15 for rx
    sw_uart_t u = sw_uart_init(14,15, 115200);

    sw_uart_putk(&u, "TRACE:WORKED:\thello from sw-uart: worked!\n");
    for(int i = 0; i < 10; i++)
        sw_uart_printk(&u, "TRACE:WORKED=%d:\thello from sw-uart!\n",i);

    // reset to using the hardware uart.
    uart_init();
    trace("if you see `WORKED` above, sw uart worked!\n");

    /*************************************************************
     * test 2: override system putc with our uart.
     */

    // set to use ours.
    output("test 2: about to override the system putchar: should see `hello world...`\n");

    sw_uart_set_putchar();

    trace("WORKED: override successful: hello using sw-uart!\n");

    hw_uart_set_putchar();

    trace("reset done: using hw uart: printed %d characters\n", putc_cnt);
    assert(putc_cnt > 0);

}
