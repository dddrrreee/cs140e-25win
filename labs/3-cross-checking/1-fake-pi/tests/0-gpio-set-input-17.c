// trivial program to set <pin> as input.  easy to 
// debug.
#include "rpi.h"

enum { pin = 17 };
void notmain(void) {
    output("setting pin %d as an input\n", pin);
    gpio_set_input(pin);
}
