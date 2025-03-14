// hard-code print hello in a simple way: 
// tests exit and putc.
//
// maybe we should set up our own stack?
#include "libos.h"

void notmain(void) {
    output("hello: pid=$pid\n");
    sys_exit(0);
}
