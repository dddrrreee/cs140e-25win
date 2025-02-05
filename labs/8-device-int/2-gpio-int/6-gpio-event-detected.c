#include "rpi.h"

void notmain(void) {
    int pin = 20;

    // if you modified <fake-pi.c> correctly, this should be 
    // changing.
    trace("event detected = %d\n", gpio_event_detected(pin));
    trace("event detected = %d\n", gpio_event_detected(pin));
    trace("event detected = %d\n", gpio_event_detected(pin));
    trace("event detected = %d\n", gpio_event_detected(pin));
    trace("event detected = %d\n", gpio_event_detected(pin));
    trace("event detected = %d\n", gpio_event_detected(pin));
    trace("event detected = %d\n", gpio_event_detected(pin));
    trace("event detected = %d\n", gpio_event_detected(pin));
}
