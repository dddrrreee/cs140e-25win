// for this test we will *not* get the same hashes, but you should get the
// same hash within a given run.  
//   1. the routines are written in C code and are in the middle of the bhinary.
//   2. they will almost certainly be at different locations given that
//      people have different implementations and different compiler versions
//      --- even a single additional instruction at the start of the binary
//      will give different results.
//   3. however, since the routines are deterministic, you should get the 
//      same results within a given run.
#include "rpi.h"
#include "eqx-threads.h"
#include "expected-hashes.h"

void sys_equiv_putc(uint8_t ch);

// print everyting out
void equiv_puts(char *msg) {
    for(; *msg; msg++)
        sys_equiv_putc(*msg);
}

void hello(void *msg) {
    equiv_puts("hello from 1\n");
}
void msg(void *msg) {
    equiv_puts(msg);
}

// these threads use stacks.
static eqx_th_t * run_single(int N, void (*fn)(void*), void *arg, uint32_t hash) {
    // run just the one thread by itself.
    let th = eqx_fork(fn, arg, hash);  
    // we turn off printing: if you need to debug enable it.
    th->verbose_p = 0;
    eqx_run_threads();
    trace("--------------done first run!-----------------\n");

    // now rerun and make sure we get the same hash.
    // turn off extra output
    // th->verbose_p = 0;
    for(int i = 0; i < N; i++) {
        eqx_refork(th);
        eqx_run_threads();
        trace("--------------done run=%d!-----------------\n", i);
    }
    return th;
}

void notmain(void) {
    eqx_init();

    // do the smallest ones first: oh, if you have a pid in the 
    // string then its nbd.
    let th1 = run_single(0, hello, 0, 0);
    let th2 = run_single(0, msg, "hello from 2\n", 0);
    let th3 = run_single(0, msg, "hello from 3\n", 0);

    // note: we turn off printing. if you need to debug you'd enable it.
    th1->verbose_p = 0;
    th2->verbose_p = 0;
    th3->verbose_p = 0;

    eqx_refork(th1);
    eqx_refork(th2);
    eqx_refork(th3);
    eqx_run_threads();
    trace("stack passed!\n");

    output("---------------------------------------------------\n");
    output("about to do quiet run\n");
    eqx_verbose(0);
    eqx_refork(th1);
    eqx_refork(th2);
    eqx_refork(th3);
    eqx_run_threads();
    trace("stack passed!\n");
}
