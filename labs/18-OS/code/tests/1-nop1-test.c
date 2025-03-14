// run N copies of the same thread together.
#include "rpi.h"
#include "eqx-threads.h"
#include "expected-hashes.h"


enum { N = 20 };

void notmain(void) {
    uint32_t h = 0;

    // running a a single nop_1 thread should give us
    // the expected hash.
    eqx_init();
    let th = eqx_fork_nostack(nop_1, 0, NOP1_HASH);
    h = eqx_run_threads();
    assert(th->reg_hash == NOP1_HASH);
    assert(h == NOP1_HASH);

    // now run N!  should also work.
    output("about to run %d nop_1 threads\n", N);
    for(int i = 0; i < N; i++)
        eqx_fork_nostack(nop_1, 0, NOP1_HASH);
    h = eqx_run_threads();
    assert(h == NOP1_HASH*20);
    trace("done!  ran %d threads, hash=%x\n", N,h);
}
