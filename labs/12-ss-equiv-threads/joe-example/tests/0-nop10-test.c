// trivial test: just run in isolation and make sure get 
// the right answer.

#include "rpi.h"
#include "eqx-threads.h"
#include "expected-hashes.h"

void notmain(void) {
    uint32_t h = 0;
    uint32_t hash = NOP10_HASH;

    // running a a single nop_1 thread should give us
    // the expected hash.
    eqx_init();
    let th = eqx_fork_nostack(nop_10, 0, hash);
    h = eqx_run_threads();
    assert(th->reg_hash == hash);
    assert(h == hash);
}
