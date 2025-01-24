#include "rpi.h"

// see broadcomm documents for magic addresses.
#define GPIO_BASE 0x20200000
static unsigned gpio_pud   = (GPIO_BASE + 0x94);
static unsigned gpio_clk0  = (GPIO_BASE + 0x98);

/*
 * p100 of broadcom gives the sequence to enable/disable pullup/pulldown.  it's
 * unfortunately very confusing, since the document indicates writing a 0 to 
 * pudclk has no effect.   also, which clock?
 *
 * open:
 *  1. how to test this?
 *  2. where did you consult to make these decisions?   was it kernel?
 */
void gpio_set_pud(unsigned pin, unsigned pud) {
    demand(pin < 32, pin too large);
    demand(pud < 4, pud too large);

    if(pin >= 32)
        panic("what");
    if(pud >= 0b11)
        panic("what");

    dev_barrier();

    // 1. write to GGPUD to set the required control signal.
    PUT32(gpio_pud, pud);
    // 2. wait 150 cycles --- this provides required set-up time for control signal.
    delay_us(150);
    // 3. write to gppudclk0 to clock the control signal into the gpio pads you want
    // to modify.  only the pads which receive the clock will be modified [i.e., no
    // r-m-w needed].
    PUT32(gpio_clk0, 1<<pin);
    // 4. wait 150 cycles.
    delay_us(150);

    // 5. write to GPPUD to remove the control signal [but says writing 0 disables
    // the pull up pull down!
    PUT32(gpio_pud, 0);
    delay_us(150);


    // 6. write to clock to remove clock.   [but states writing 0 has no effect!]
    PUT32(gpio_clk0, 0);
    delay_us(150);

    dev_barrier();
}

void gpio_set_pullup(unsigned pin) { gpio_set_pud(pin, 0b10); }
void gpio_set_pulldown(unsigned pin) { gpio_set_pud(pin, 0b01); }
void gpio_pud_off(unsigned pin) { gpio_set_pud(pin, 0b00); }

int gpio_get_pud(unsigned pin) {
    return (GET32(gpio_pud) & 0b11);
} 

enum { GPIO_PULLUP = 0b10, GPIO_PULLDOWN = 0b01 };

int gpio_is_pullup(unsigned pin) {
    return gpio_get_pud(pin) == GPIO_PULLUP;
}
int gpio_is_pulldown(unsigned pin) {
    return gpio_get_pud(pin) == GPIO_PULLDOWN;
}

#if 0
// does not seem to be a way to read from pud.
// return the current state of <pin>
int gpio_is_pullup(unsigned pin) {
    return gpio_get_pud(pin); 
    return GET32(gpio_pud) ;
    return (GET32(gpio_pud) & 0b11) == 0b10;
}
int gpio_is_pulldown(unsigned pin) {
    return (GET32(gpio_pud) & 0b11) == 0b01;
}
int gpio_is_pulloff(unsigned pin) {
    return (GET32(gpio_pud) & 0b11) == 0b00;
}
#endif
