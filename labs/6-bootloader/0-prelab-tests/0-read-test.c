#include <string.h>
#include "libunix.h"

// see which fd the system returns.
static int fd_next(void) {
    const char *filename = "./emptyxxx.c";

    int fd = create_file(filename);
    unlink(filename);
    close(fd);
    return fd;
}

int main(void) {
    const char *name = "./read-file.out";
    const char *hello = "hello";
    
    int fd = create_file(name);
    int i;
    for(i = 0; hello[i]; i++)
        write_exact(fd, &hello[i], 1);
    // write 0
    write_exact(fd, &hello[i], 1);
    close(fd);

    // make sure we always get the same fd after closing.
    if(fd_next() != fd_next())
        panic("your OS behaves weirdly, let us know!\n");  
    if(fd_next() != fd_next())
        panic("your OS behaves weirdly, let us know!\n");  
    if(fd_next() != fd_next())
        panic("your OS behaves weirdly, let us know!\n");  

    // check that <read_file> does not lose file descriptor.
    int fd_before = fd_next();

    unsigned nbytes;
    char *data = read_file(&nbytes, name);

    int fd_after = fd_next();
    if(fd_before != fd_after)
        panic("it appears <read_file> leaked a file descriptor\n");

    unsigned n = strlen(hello) + 1;
    if(nbytes != n)
        panic("expected %d bytes, have %d\n", n, nbytes);
    else
        trace("success: expected %d bytes, have %d\n", n, nbytes);

    if(strcmp(hello, data) != 0)
        panic("expected string <%s> have <%s>\n", hello,data);
    else
        trace("expected string <%s> have <%s>\n", hello,data);

    // make sure trailing bytes are 0.
    assert(data[7] == 0);
    assert(data[8] == 0);

    run_system("rm -f %s", name);
    trace("read-file: success [should test other sizes!]\n");
    return 0;
}
