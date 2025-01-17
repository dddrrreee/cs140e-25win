#include "rpi.h"

void notmain(void) {
    output("hello\n");
    clean_reboot();
}
