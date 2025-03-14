// hard-code print hello in a simple way: 
// tests exit and putc.
//
// maybe we should set up our own stack?
#include "libos.h"

void notmain(void) {
    int pid = sys_fork();

    if(!pid) {
        debug("child pid=$pid\n");
        sys_exit(0xdeadbeef);
    } else {
        debug("parent pid=$pid, child=%d!\n", pid);
        uint32_t res = sys_waitpid(pid, WNOHANG);
        debug("parent pid=$pid, child returned: %x!\n", pid, res);
    }
    sys_exit(0);
}
