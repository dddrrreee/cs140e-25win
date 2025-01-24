// engler, cs140 put your gpio implementations in here.
#include "rpi.h"
#include "rpi-interrupts.h"

enum {
    GPIO_BASE = 0x20200000,
    // define
    //  GPIO_EVENT_DETECT0
    //  GPIO_RISING_EDGE
    //  GPIO_FALLING_EDGE
    //  GPIO_HIGH_ENABLED0
    //  GPIO_LOW_ENABLED0
    GPIO_EVENT_DETECT0 = GPIO_BASE+0x40,
    GPIO_RISING_EDGE = GPIO_BASE + 0x4C,
    GPIO_FALLING_EDGE = GPIO_BASE + 0x58,
    GPIO_HIGH_DETECT = GPIO_BASE+0x64,
    GPIO_LOW_DETECT = GPIO_BASE+0x70,
};

// enum { GPIO_INT0 = 49, GPIO_INT1, GPIO_INT2, GPIO_INT3 };

#if 0
static void or32(volatile void *addr, uint32_t val) {
    put32(addr, get32(addr) | val);
}
static void OR32(unsigned long addr, uint32_t val) {
    or32((volatile void*)addr, val);
}
#endif


void gpio_int_set(unsigned addr, unsigned pin) {
    if(pin >= 32)
        return;

    dev_barrier();
    OR32(addr, 1 << pin);
    dev_barrier();

    // p 113: have to enable a gpio_int[0], which = 49.  This means we use enable_IRQs_2:
    //  - Enable_IRQs_1 covers [0..31],
    //  - Enable_IRQs_2 covers [32..63]
    unsigned off = GPIO_INT0 % 32;
    PUT32(Enable_IRQs_2, 1 << off);
    // printk("off=%d\n", off);

    dev_barrier();
}

// gpio_int_rising_edge and gpio_int_falling_edge (and any other) should
// call this routine (you must implement) to setup the right GPIO event.
// as with setting up functions, you should bitwise-or in the value for the 
// pin you are setting with the existing pin values.  (otherwise you will
// lose their configuration).  you also need to enable the right IRQ.   make
// sure to use device barriers!!
int gpio_has_interrupt(void) {
    // assert(gpio_int >= GPIO_INT0 && gpio_int <= GPIO_INT3);
    unsigned gpio_int = GPIO_INT0;
    unsigned pending = GET32(IRQ_pending_2);
    unsigned off = gpio_int % 32;
    return (pending >> off) & 1;
}

// p97 set to detect rising edge (0->1) on <pin>.
// as the broadcom doc states, it  detects by sampling based on the clock.
// it looks for "011" (low, hi, hi) to suppress noise.  i.e., its triggered only
// *after* a 1 reading has been sampled twice, so there will be delay.
// if you want lower latency, you should us async rising edge (p99)
void gpio_int_rising_edge(unsigned pin) {
    if(pin>=32)
        return;
    gpio_int_set(GPIO_RISING_EDGE, pin);
}

// p98: detect falling edge (1->0).  sampled using the system clock.  
// similarly to rising edge detection, it suppresses noise by looking for
// "100" --- i.e., is triggered after two readings of "0" and so the 
// interrupt is delayed two clock cycles.   if you want  lower latency,
// you should use async falling edge. (p99)
void gpio_int_falling_edge(unsigned pin) {
    if(pin>=32)
        return;
    gpio_int_set(GPIO_FALLING_EDGE, pin);
}


void gpio_int_async_falling_edge(unsigned pin) {
    if(pin>=32)
        return;
    unimplemented();
    //gpio_int_set(GPIO_ASYNC_FALLING_EDGE, pin);
}
void gpio_int_async_rising_edge(unsigned pin) {
    if(pin>=32)
        return;
    unimplemented();
    // gpio_int_set(GPIO_ASYNC_RISING_EDGE, pin);
}



void gpio_enable_hi_int(unsigned pin) {
    if(!pin || pin>=32)
        panic("invalid pin: %d\n", pin);
    gpio_int_set(GPIO_HIGH_DETECT, pin);
}

void gpio_enable_lo_int(unsigned pin) {
    if(!pin || pin>=32)
        panic("invalid pin: %d\n", pin);
    gpio_int_set(GPIO_LOW_DETECT, pin);
}


// p96: a 1<<pin is set in EVENT_DETECT if <pin> triggered an interrupt.
// if you configure multiple events to lead to interrupts, you will have to 
// read the pin to determine which caused it.
int gpio_event_detected(unsigned pin) {
    if(pin >= 32)
        return 0;
    dev_barrier();
    unsigned r = (GET32(GPIO_EVENT_DETECT0) >> pin) & 1;
    dev_barrier();
    return r;
}

// p96: have to write a 1 to the pin to clear the event.
void gpio_event_clear(unsigned pin) {
    if(pin >= 32)
        return;
    dev_barrier();
    PUT32(GPIO_EVENT_DETECT0, 1 << pin);
    dev_barrier();
}

void using_staff_gpio_int(void){}
