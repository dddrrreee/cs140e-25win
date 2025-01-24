/*
 * reimplement your sw-uart code below using cycles.   helpers are in <cycle-util.h>
 */ 
#include "rpi.h"
#include "sw-uart.h"
#include "cycle-util.h"

#include <stdarg.h>
// implement te putc/getc/readuntil.

#if 0
// write val <v> to gpio pin <pin> and delay until <ncycles>
// have passed starting from <start_cycle>
// use 
static void inline gpio_write_ncyc(
    unsigned pin, 
    unsigned v,  // 0 or 1.
    uint32_t start_cycle,  
    uint32_t ncycles 
{

}

static inline uint32_t abs(int32_t x) {
    return (x<0) ? -x : x;
}
    
// measure the absolute error between expected number
// of cycles that <fn> should take vs what it actually
// took.  
//
// note: this won't be zero b/c of measurement overhead.
#define measure_err(err, fn, expect_n) do {                     \
    /* 1. measure the actual cycles <fn> takes. */              \
    uint32_t s = cycle_cnt_read();                              \
    fn;                                                         \
    uint32_t e = cycle_cnt_read();                              \
    /* total cycles = end - start */                            \
    uint32_t measured_n = e - s;                                \
    /* 2. error is absolute value of the difference */          \
    err += abs(expect_n - measured_n);                          \
} while(0)
#endif


// do this first: used timed_write to cleanup.
//  recall: time to write each bit (0 or 1) is in <uart->usec_per_bit>
void sw_uart_put8(sw_uart_t *uart, uint8_t c) {
    // use local variables to minimize any loads or stores
    int tx = uart->tx;
    uint32_t n = uart->cycle_per_bit,
             u = n,
             s = cycle_cnt_read();

    // unimplemented();
    write_cyc_until(tx, 0, s, u);      u+=n;      // start bit

        write_cyc_until(tx, (c>>0) & 1, s, u);  u += n;
        write_cyc_until(tx, (c>>1) & 1, s, u);  u += n;
        write_cyc_until(tx, (c>>2) & 1, s, u);  u += n;
        write_cyc_until(tx, (c>>3) & 1, s, u);  u += n;
        write_cyc_until(tx, (c>>4) & 1, s, u);  u += n;
        write_cyc_until(tx, (c>>5) & 1, s, u);  u += n;
        write_cyc_until(tx, (c>>6) & 1, s, u);  u += n;
        write_cyc_until(tx, (c>>7) & 1, s, u);  u += n;

    write_cyc_until(tx, 1, s, u);                  // stop bit.
}

// do this second: you can type in pi-cat to send stuff.
//      EASY BUG: if you are reading input, but you do not get here in 
//      time it will disappear.
int sw_uart_get8_timeout(sw_uart_t *uart, uint32_t timeout_usec) {
   int rx = uart->rx;

    // get start bit: timeout_usec=0 implies you return 
    // right away (used to be return never).
    while(!wait_until_usec(rx, 0, timeout_usec))
        return -1;

    unsigned s = cycle_cnt_read();  // subtract off slop?

    uint32_t u = uart->cycle_per_bit;
    unsigned n = u/2;
    unsigned c = 0;

    // wait one period + 1/2 to get in the middle of the next read.
    delay_ncycles(s, n + 1*u);
    c |= GPIO_READ_RAW(rx) << 0;
    delay_ncycles(s, n + 2*u);
    c |= GPIO_READ_RAW(rx) << 1;
    delay_ncycles(s, n + 3*u);
    c |= GPIO_READ_RAW(rx) << 2;
    delay_ncycles(s, n + 4*u);
    c |= GPIO_READ_RAW(rx) << 3;
    delay_ncycles(s, n + 5*u);
    c |= GPIO_READ_RAW(rx) << 4;
    delay_ncycles(s, n + 6*u);
    c |= GPIO_READ_RAW(rx) << 5;
    delay_ncycles(s, n + 7*u);
    c |= GPIO_READ_RAW(rx) << 6;
    delay_ncycles(s, n + 8*u);
    c |= GPIO_READ_RAW(rx) << 7;

    // delay_ncycles(s, n + 8*u);

    // wait until we get <1> or until deadline expires.
    wait_until_cyc(rx, 1, s, n+9*u);
    return (int)c;
}

// finish implementing this routine.  
sw_uart_t sw_uart_init_helper(unsigned tx, unsigned rx,
        unsigned baud, 
        unsigned cyc_per_bit,
        unsigned usec_per_bit) {

    assert(tx && tx<31);
    assert(rx && rx<31);
    assert(cyc_per_bit && cyc_per_bit > usec_per_bit);
    assert(usec_per_bit);

    // maybe enable the cache?  don't think will work otherwise.
    gpio_set_input(rx);
    gpio_set_output(tx);

    // easy bug: must do this.
    gpio_write(tx,1);

    // make sure it makes sense.
    unsigned mhz = 700 * 1000 * 1000;
    unsigned derived = cyc_per_bit * baud;
    if(! ((mhz - baud) <= derived && derived <= (mhz + baud)) )
        panic("too much diff: cyc_per_bit = %d * baud = %d\n", 
            cyc_per_bit, cyc_per_bit * baud);

    return (sw_uart_t) { 
            .tx = tx, 
            .rx = rx, 
            .baud = baud, 
            .cycle_per_bit = cyc_per_bit ,
            .usec_per_bit = usec_per_bit 
    };
}

#if 0
// trivial putk.
static inline void 
sw_uart_putk(sw_uart_t *uart, const char *msg) {
    for(; *msg; msg++)
        sw_uart_put8(uart, *msg);
}
#endif
