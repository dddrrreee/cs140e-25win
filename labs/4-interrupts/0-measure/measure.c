// measure the cost (in cycles) of different operations.
// good way to get intuition, and to see how it breaks.
//
// NOTE: we mostly only measure one op at a time without 
// handling alignment or cache effects --- worth seeing
// how clean you can get the readings.
#include "rpi.h"
#include "cycle-count.h" // libpi/include/cycle-count.h

void measure(void);
void measure_weird(void);

void notmain(void) {
    printk("\nmeasuring cost of different operations (pi A+ = 700 cyc / usec)\n");
    cycle_cnt_init();

    printk("without caching:\n");
    measure();

    caches_enable();
    printk("with icache on first run\n");
    measure();
    printk("with icache on second run\n");
    measure();
}
    

void measure(void) {
    // simple example to measure PUT32
    let s = cycle_cnt_read();
    PUT32(0,0);
    let e = cycle_cnt_read();
    printk("\tcall to PUT32=%d cycles\n", e-s);

    // simple example to measure GET32
    s = cycle_cnt_read();
    GET32(0);
    e = cycle_cnt_read();
    printk("\tcall to GET=%d cycles\n", e-s);

    // a bit weird :)
    s = cycle_cnt_read();
    *(volatile uint32_t *)0 = 4;
    e = cycle_cnt_read();
    printk("\tnull pointer write =%d cycles\n", e-s);

    asm volatile(".align 4");
    s = cycle_cnt_read();
    *(volatile uint32_t *)0 = 4;
    e = cycle_cnt_read();
    printk("\taligned null pointer write =%d cycles\n", e-s);

    // use macro <TIME_CYC_PRINT> to clean up a bit.
    //  see: libpi/include/cycle-count.h
    TIME_CYC_PRINT("\tread/write barrier", dev_barrier());
    TIME_CYC_PRINT("\tread barrier", dmb());
    TIME_CYC_PRINT("\tsafe timer", timer_get_usec());
    TIME_CYC_PRINT("\tunsafe timer", timer_get_usec_raw());
    // macro expansion messes up the printk
    printk("\t<cycle_cnt_read()>: %d\n", TIME_CYC(cycle_cnt_read()));

    // seperate out so it's easier to see.
    measure_weird();
    printk("-----------------------------------------\n");

}

// measure a weirdness with alignment.
void measure_weird(void) {
    printk("\n\nmeasuring a weird feature:\n");

    // 8 cycles
    asm volatile(".align 4");
    let s = cycle_cnt_read();
    let e = cycle_cnt_read();
    printk("\tempty cycle count = measurement: %d cycles\n", e-s);

    asm volatile(".align 4");
    asm volatile("mov r0,r0");  // nop
    asm volatile("mov r0,r0");
    asm volatile("mov r0,r0");

    // about 41: "why??"
    s = cycle_cnt_read();
    e = cycle_cnt_read();
    printk("\tempty cycle count = measurement: %d cycles\n", e-s);
}
