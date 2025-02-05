#include "rpi.h"

void notmain() {
    trace("checking that loopback works: connect 20,21\n");
    gpio_set_output(20);
    gpio_set_input(21);


    trace("writing 1 to 20\n");
    gpio_write(20,1);

    if(gpio_read(21) != 1)
        panic("21 is 0: check loopback\n");
    else
        trace("success: read 1!\n");


    gpio_write(20,0);
    if(gpio_read(21) != 0)
        panic("21 is 0: check loopback\n");
    else
        trace("success: read 0!\n");
    trace("SUCCESS: test passed!\n");
}
