// blink the on-board "act" led, 47.  this will check
// that you handle higher numbered LEDs.
#include "rpi.h"

// ACT is the green LED on the pi zero board itself.
enum { act_led = 47 };
void act_init(void) { gpio_set_output(act_led); }
// NOTE: off/on are reversed.
void act_on(void) { gpio_write(act_led,0); }
void act_off(void) { gpio_write(act_led,1); }

void notmain(void) {
    act_init();
    for(int i = 0; i < 10; i++) {
        act_on();
        delay_cycles(1000000);
        act_off();
        delay_cycles(1000000);
    }
}
