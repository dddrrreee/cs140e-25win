#include "rpi.h"
void notmain(void) {
    output("about to do a single read from pins [0..32)\n");
    for(unsigned pin = 0; pin < 32; pin++) {
        output("reading from pin %d\n", pin);
        gpio_read(pin);
    }
}
