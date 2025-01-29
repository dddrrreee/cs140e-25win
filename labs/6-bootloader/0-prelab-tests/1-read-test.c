// check that read_file works on all buffers from size [0..N)
#include <string.h>
#include "libunix.h"

void read_check(unsigned char *buf, unsigned n) {
    char *name = "test-file.c";

    int fd = create_file(name);
    if(n)
        write_exact(fd, buf,n);
    close(fd);

    unsigned nbytes;
    unsigned char *data = read_file(&nbytes, name);
    assert(data);

    // check that the contents match.
    unsigned i;
    for(i = 0; i < n; i++)
        if(data[i] != buf[i])
            panic("read_file: %d nbytes differs at offset %d: expect=%x, got=%x\n", n,i, buf[i], data[i]);

    // does it zero pad?
    unsigned rem = pi_roundup(n,4);
    for(; i < rem; i++)
        if(data[i])
            panic("read_file: %d nbytes did not zero pad at offset %d\n", n,i);
        else
            trace("read_file(n=%d): off=%d has correct zero pad\n", n,i);

    unlink(name);
    free(data);
}

int main(void) {
    enum { N = 1024 };
    unsigned char *mem = malloc(N);

    for(int i = 0; i < N; i++)
        mem[i] = random();

    for(int i = 0; i < N; i++)
        read_check(mem,i);

    trace("success: tested %d read_file calls\n", N);
}
