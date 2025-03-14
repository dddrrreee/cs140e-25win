#include "libos.h"

// we put this here so that adding system calls doesn't change
// the hashes.

void libos_cstart() {
        void notmain();
        notmain();
        sys_exit(0);
}
