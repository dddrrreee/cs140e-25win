#include "rpi.h"
void notmain(void) {
    output("about to turn pins [0..32) on\n");
    for(unsigned pin = 0; pin < 32; pin++) {
        output("turning pin %d on\n", pin);
        gpio_write(pin,1);
    }

    output("about to turn pins [0..32) off\n");
    for(unsigned pin = 0; pin < 32; pin++) {
        output("turning pin %d off\n", pin);
        gpio_write(pin,0);
    }
}
