// set pins [0..32) as input/output.
#include "rpi.h"
void notmain(void) {

    // note: this won't work for uart on raw hw.
    for(unsigned pin = 0; pin < 32; pin++) {
        output("setting pin %d as input\n", pin);
        gpio_set_input(20);
    }

    for(unsigned pin = 0; pin < 32; pin++) {
        output("setting pin %d as output\n", pin);
        gpio_set_output(20);
    }
}
