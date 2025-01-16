
// set the act pin as output
#include "rpi.h"

enum { pin = 47 };
void notmain(void) {
    output("about to set act as output\n");
    gpio_set_output(pin);
}
