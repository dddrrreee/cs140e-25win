// run threads all at once: should see their pids smeared.
#include "rpi.h"
#include "eqx-threads.h"
#include "expected-hashes.h"

void sys_equiv_putc(uint8_t ch);

// print everyting out
void equiv_puts(char *msg) {
    for(; *msg; msg++)
        sys_equiv_putc(*msg);
}

// simple thread that just prints its argument as a string.
void msg_fn(void *msg) {
    equiv_puts(msg);
}

void notmain(void) {
    eqx_verbose(0);
    eqx_init();
    
    enum { N = 8 };
    eqx_th_t *th[N];

    // run each thread by itself.
    for(int i = 0; i < N; i++) {
        void *buf = kmalloc(256);
        snprintk(buf, 256, "hello from %d\n", i+1);
        th[i] = eqx_fork(msg_fn, buf, 0);
        th[i]->verbose_p = 0;
        eqx_run_threads();
    }

    output("---------------------------------------------------\n");
    output("about to do quiet run\n");
    eqx_verbose(0);

    // refork and run all together.
    for(int i = 0; i < N; i++)
        eqx_refork(th[i]);
    eqx_run_threads();
}
