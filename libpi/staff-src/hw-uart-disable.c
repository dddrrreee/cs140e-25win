#include "rpi.h"

#define ENABLE_UART 0x1
static volatile uint32_t *const aux_enables = (void*)0x20215004;


static void and_in32(volatile void *addr, unsigned val) {
    put32(addr, get32(addr) & val);
}

void hw_uart_disable(void) {
    uart_flush_tx();
    dev_barrier();
    and_in32(aux_enables, ~ENABLE_UART);
    dev_barrier();
}

