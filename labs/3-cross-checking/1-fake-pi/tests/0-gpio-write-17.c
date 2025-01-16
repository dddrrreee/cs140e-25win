// trivial: we do a single read of pin 17.
// should be easy to debug!
#include "rpi.h"

enum { pin = 17 };
void notmain(void) {
    output("about to turn pin=%d on\n", pin);
    gpio_write(pin,1);
    output("about to turn pin=%d off\n", pin);
    gpio_write(pin,0);
}
