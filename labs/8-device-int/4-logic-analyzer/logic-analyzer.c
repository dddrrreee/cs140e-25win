/*
 * engler, cs140e: simple tests to check that you are handling rising and falling
 * edge interrupts correctly.
 *
 * NOTE: make sure you connect your GPIO 20 to your GPIO 21 (i.e., have it "loopback")
 */
#include "rpi.h"
#include "rpi-interrupts.h"
#include "libc/circular.h"
#include "sw-uart.h"
#include "cycle-count.h"

// simple circular queue, suitable for interrupt
// concurrency.
//  libpi/libc/circular.h
static cq_t uartQ;

enum { out_pin = 21, in_pin = 20 };
static volatile unsigned n_rising_edge, n_falling_edge;

// similar to our timer interrupt vector but for GPIO.
// can tune significantly!
void interrupt_vector(unsigned pc) {
    // 1. compute the cycles since the last event and push32 in the queue
    // 2. push the previous pin value in the circular queue (so if was
    //    falling edge: previous must have been a 1).
    //
    // notes:
    //  - make sure you clear the GPIO event!
    //  - using the circular buffer is pretty slow. should tune this.
    //    easy way is to use a uint32_t array where the counter is volatile.
    unsigned s = cycle_cnt_read();

    dev_barrier();
    unimplemented();
    dev_barrier();
}

void notmain() {
    cq_init(&uartQ,1);

    // libpi/include/rpi-interrupt.h: initialize
    // default vectors.
    interrupt_init();

    // enable icache and branch target buffer (predict)
    caches_enable();

    // use pin 20 for tx, 21 for rx
    sw_uart_t u = sw_uart_init(out_pin,in_pin, 115200);

    gpio_int_rising_edge(in_pin);
    gpio_int_falling_edge(in_pin);
    gpio_event_clear(in_pin);

    // enable interrupts.
    uint32_t cpsr = cpsr_int_enable();

    assert(gpio_read(in_pin) == 1);

    // starter code.
    // make sure this works first, then try to measure the overheads.
    delay_ms(100);

    output("expect each read to take around %d cycles\n", 
                u.cycle_per_bit);

    // this will cause transitions every time, so you can compare times.
    for(int l = 0; l < 2; l++) {
        // note: uart start bit = 0, stop bit = 1;
        uint32_t b = 0b01010101;
        sw_uart_put8(&u, b);
        delay_ms(100);
        printk("nevent=%d\n", cq_nelem(&uartQ)/(4*2));

        if(cq_empty(&uartQ)) 
            panic("circular queue is empty?\n");

        // subtract the first reading to get the difference.
        uint32_t last = cq_pop32(&uartQ);
        cq_pop32(&uartQ);
        unsigned nbit = 0;
        while(!cq_empty(&uartQ)) {
            unsigned ncycles    = cq_pop32(&uartQ);
            unsigned v          = cq_pop32(&uartQ);
            printk("\tnbit=%d: v=%d, cyc=%d\n", 
                    nbit, v, ncycles - last);
            last = ncycles;
            nbit++;
        }
    }
    trace("SUCCESS!\n");
}
