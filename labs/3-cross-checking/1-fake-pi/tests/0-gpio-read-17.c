// trivial: we do a single read of pin 17.
// should be easy to debug!
#include "rpi.h"
void notmain(void) {
    output("about to read 17\n");
    gpio_read(17);
}
