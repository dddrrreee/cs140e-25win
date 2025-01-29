#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "libunix.h"

#define _SVID_SOURCE
#include <dirent.h>

int main(int argc, char *argv[]) {
    if(argc < 2)
        panic("no program given\n");
    const char *prog = argv[1];

    for(int i = 2; i < argc; i++) {
        const char *p = argv[i];
        char buf[8192];
        char *q = buf;
    
        while(*p) {
            if(*p == ' ' || *p == ')' || *p == '(')
                *q++ = '\\';
            *q++ = *p++;
        }
        *q++ = 0;
        output("about to run <%s %s>\n", prog,buf);
        run_system("%s %s", prog, buf);
    }
    return 0;
}
