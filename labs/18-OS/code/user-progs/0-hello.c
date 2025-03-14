// hard-code print hello in a simple way: 
// tests exit and putc.
//
// maybe we should set up our own stack?
#include "libos.h"

void notmain(void) {
    sys_putc('h');
    sys_putc('e');
    sys_putc('l');
    sys_putc('l');
    sys_putc('o');
    sys_putc('\n');
    sys_exit(0);
}
