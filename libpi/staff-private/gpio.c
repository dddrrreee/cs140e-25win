/*
 * write code to allow blinking using arbitrary pins.    
 * Implement:
 *	- gpio_set_output(pin) --- set GPIO <pin> as an output (vs input) pin.
 *	- gpio_set_on(pin) --- set the GPIO <pin> on.
 * 	- gpio_set_off(pin) --- set the GPIO <pin> off.
 * Use the minimal number of loads and stores to GPIO memory.  
 *
 * start.s defines a of helper functions (feel free to look at the assembly!  it's
 *  not tricky):
 *      uint32_t get32(volatile uint32_t *addr) 
 *              --- return the 32-bit value held at <addr>.
 *
 *      void put32(volatile uint32_t *addr, uint32_t v) 
 *              -- write the 32-bit quantity <v> to <addr>
 * 
 * Check-off:
 *  1. get a single LED to blink.
 *  2. attach an LED to pin 19 and another to pin 20 and blink in opposite order (i.e.,
 *     one should be on, while the other is off).   Note, if they behave weirdly, look
 *     carefully at the wording for GPIO set.
 */
#include "rpi.h"

enum {
    // see broadcomm documents for magic addresses.
    GPIO_BASE  = 0x20200000,

    gpio_set0  = (GPIO_BASE + 0x1C),
    gpio_clr0  = (GPIO_BASE + 0x28),
    gpio_lev0  = (GPIO_BASE + 0x34),

    gpio_set1  = gpio_set0 + 4,
    gpio_clr1  = gpio_clr0 + 4,
};

// could parameterize panic to to blink the ACT led if no UART.
// can check the uart itself.  or have the uart swap things.
//
// is probably better to panic otherwise they are going to have
// weird errors.  could have them add?
 
#if 0
static void or32(volatile void *addr, unsigned v) {
    put32(addr, get32(addr) | v);
}
#endif

// set <pin> to output.  note: fsel0, fsel1, fsel2 are contiguous in memory,
// so you can use array calculations!

void gpio_set_function(unsigned pin, gpio_func_t func) {
    if(pin >= 32 && pin != 47)
        return;
    if((func & 0b111) != func)
        return;
    unsigned off = (pin%10)*3;
    unsigned g = GPIO_BASE + (pin/10)*4;

    unsigned v = GET32(g) ;
    v &= ~(0b111 << off);
    v |= func << off;
    PUT32(g,v);
}

void gpio_set_output(unsigned pin) {
    gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}

// set <pin> on.
void gpio_set_on(unsigned pin) {
    if(pin == 47)
        PUT32(gpio_set1, 1 << pin % 32);
    else {
        if(pin >= 32)
            return;
        PUT32(gpio_set0, 1 << pin);
    }
}

// set <pin> off
void gpio_set_off(unsigned pin) {
    if(pin == 47)
        PUT32(gpio_clr1, 1 << pin % 32);
    else {
        if(pin >= 32)
            return;
        PUT32(gpio_clr0, 1 << pin);
    }
}

// set <pin> to input.
void gpio_set_input(unsigned pin) {
    gpio_set_function(pin, GPIO_FUNC_INPUT);
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if(v)
        gpio_set_on(pin);
    else
        gpio_set_off(pin);
}

// note: can't read from 47.
int gpio_read(unsigned pin) {
    if(pin >= 32)
        return -1;
    uint32_t x = (GET32(gpio_lev0) >> pin) & 1;
    return DEV_VAL32(x);
}

void using_staff_gpio(void){}
