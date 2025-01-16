// trivial: we do a single read of pin 21.
// should be easy to debug!
#include "rpi.h"
void notmain(void) {
    output("about to read 21\n");
    gpio_read(21);
}
