#ifndef __LIBUNIX_EMU_H__
#define __LIBUNIX_EMU_H__
#include "libos.h"

typedef long pid_t;
static inline pid_t fork(void) {
    return sys_fork();
}

static inline void exit(int status) {
    sys_exit(status);
}

// this needs to get fixed.
// 0 = wait for any child [how do we keep track?]
static inline pid_t 
waitpid(pid_t pid, int *status, uint32_t options) {
    // handle WNOHANG.
    if(options != 0)
        panic("not handling other options right now.\n");

    // this isn't actually correct: need to fix this stuff.
    *status = sys_waitpid(pid, options);
    return 0;
} 

#endif
