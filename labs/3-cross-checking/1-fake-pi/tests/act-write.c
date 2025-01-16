// write to the act pin.
#include "rpi.h"

enum { pin = 47 };
void notmain(void) {
    output("about to write 1 to act\n");
    gpio_write(pin,1);
    output("about to write 0 to act\n");
    gpio_write(pin,0);
}
