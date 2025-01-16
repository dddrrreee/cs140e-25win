// 1. start pulling in more routines (printk, clean_reboot)
// 2. use the real <rpi.h> in libpi: this means we reduce the
//    chance of infidelity.
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "rpi.h"

int printk(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
        int res = vprintf(fmt, ap);
    va_end(ap);

    return res;
}

void clean_reboot(void) {
    printk("DONE!!!\n");
    exit(0);
}

int main(void) {
    notmain();
    return 0;
}
