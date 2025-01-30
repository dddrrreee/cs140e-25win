// simple mini-uart driver.
// implement every routine with a <todo>.
//
// NOTE: you can call our staff code.
#include "rpi.h"

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void) {
    todo("must implement\n");
}

// disable the uart.
void uart_disable(void) {
    todo("must implement\n");
}

// returns one byte from the RX (input) hardware
// FIFO.  if FIFO is empty, blocks until there is 
// at least one byte.
int uart_get8(void) {
    todo("must implement\n");
}

// returns 1 if the hardware TX (output) FIFO has room
// for at least one byte.  returns 0 otherwise.
int uart_can_put8(void) {
    todo("must implement\n");
}

// put one byte on the TX FIFO, if necessary, waits
// until the FIFO has space.
int uart_put8(uint8_t c) {
    todo("must implement\n");
    return 1;
}

// returns:
//  - 1 if at least one byte on the hardware RX FIFO.
//  - 0 otherwise
int uart_has_data(void) {
    todo("must implement\n");
}

// returns:
//  -1 if no data on the RX FIFO.
//  otherwise reads a byte and returns it.
int uart_get8_async(void) { 
    if(!uart_has_data())
        return -1;
    return uart_get8();
}

// returns:
//  - 1 if TX FIFO empty
//  - 0 if not empty.
int uart_tx_is_empty(void) {
    todo("must implement\n");
}

// wait until the TX fifo is empty.  used when
// rebooting or turning off the UART to make sure
// that any output on it gets sent before (otherwise
// can be truncated).
void uart_flush_tx(void) {
    while(!uart_tx_is_empty())
        ;
}
