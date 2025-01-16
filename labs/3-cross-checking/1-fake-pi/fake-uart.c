#include "rpi.h"

// if you don't use the uart in lab 5, have fake versions.
int uart_put8(uint8_t c) {
    
    putchar(c);
    return c;
}

void uart_flush_tx(void) {
    fflush(stdout);
}

