// trivial test: just run in isolation and make sure get 
// the right answer.
#include "rpi.h"
#include "eqx-threads.h"
#include "expected-hashes.h"

void notmain(void) {
    uint32_t h = 0;

    // running a a single nop_1 thread should give us
    // the expected hash.
    eqx_init();
    let th = eqx_fork_nostack(nop_1, 0, NOP1_HASH);
    h = eqx_run_threads();
    assert(th->reg_hash == NOP1_HASH);
    assert(h == NOP1_HASH);
}
