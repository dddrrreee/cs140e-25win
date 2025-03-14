#include "libunix.h"

void notmain(void) {
    unsigned sum = 0;
    for(int i = 0; i < 8; i++) {
        int pid;

        if(!(pid = fork())) {
            output("child pid=$pid\n");
            exit(i);
        } else {
            // should take the hash of the parent?
            output("parent pid=$pid, child pid=%d\n", pid);
            int status,res = waitpid(pid,&status,0);
            if(res < 0)
                panic("waitpid failed\n");

            output("waitpid=%d\n", status);
            sum += status;
            if(status != i)
                panic("got=%d, expected=%d\n", status, i);
        }
    }
    output("SUCCESS: sum=%d\n", sum);
    exit(0);
}
