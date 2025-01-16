// trivial fake-pi: just call notmain()  and redefine
// a couple routines in <rpi.h> in this directory.
//
// can't simulate much, but can simulate the hello world
// program
#include "rpi.h"

int main(void) {
    notmain();
    return 0;
}
