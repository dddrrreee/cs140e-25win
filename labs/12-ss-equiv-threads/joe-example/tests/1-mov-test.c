// run N copies of the same thread together.
#include "rpi.h"
#include "eqx-threads.h"
#include "expected-hashes.h"


enum { N = 20 };

void notmain(void) {
    uint32_t h = 0;
    uint32_t hash = MOV_IDENT_HASH;

    // running a a single nop_1 thread should give us
    // the expected hash.
    eqx_init();
    let th = eqx_fork_nostack(mov_ident, 0, hash);
    h = eqx_run_threads();
    assert(th->reg_hash == hash);
    assert(h == hash);

    // now run N!  should also work.
    output("about to run %d nop_10 threads\n", N);
    for(int i = 0; i < N; i++)
        eqx_fork_nostack(mov_ident, 0, hash);
    h = eqx_run_threads();
    assert(h == hash*20);
    trace("done!  ran %d threads, hash=%x\n", N,h);
}
