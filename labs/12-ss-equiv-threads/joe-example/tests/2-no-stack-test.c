// run tests that should give the same result for everyone.
//   1. all the functions we run and hash their registers are defined
//      in staff-start.S in this directory.
//   2. everyone's assembler will convert this to the same machine code
//      (as far as i have seen)
//   3. we link this file as the first one in every binary.
//   4. therefore: the code will be at the same location for everyone.
//   5. therefore: the hashes will be the same --- same instructions, at 
//      same location, run on same initial state = same result.
//   this use of determinism let's us cross check our results across
//   everyone and across time.
//
//   fwiw, determinism tricks like this have gotten me/my group a lot 
//   of good results.  keep an eye out for such things as you go through
//   life.

#include "rpi.h"
#include "eqx-threads.h"
#include "expected-hashes.h"

// runs routines <N> times, comparing the result against <hash>.
// they are stateless, so each time we run them we should get
// the same result.  note: if you set hash to 0, it will run,
// and then write the result into the thread block.
static eqx_th_t * 
run_single(int N, void (*fn)(void*), void *arg, uint32_t hash) {
    let th = eqx_fork_nostack(fn, arg, hash);
    th->verbose_p = 1;
    eqx_run_threads();

    if(hash && th->reg_hash != hash)
        panic("impossible: eqx did not catch mismatch hash\n");
    hash = th->reg_hash;
    trace("--------------done first run!-----------------\n");

    // turn off extra output
    // th->verbose_p = 0;
    for(int i = 0; i < N; i++) {
        eqx_refork(th);
        eqx_run_threads();
        if(th->reg_hash != hash)
            panic("impossible: eqx did not catch mismatch hash\n");
        trace("--------------done run=%d!-----------------\n", i);
    }
    return th;
}

void notmain(void) {
    eqx_init();

    //  strategy: i would run just the first routine first and comment out
    //  the rest.  make sure it works.  then do the next, then the next etc.

    // do the smallest ones first.
    let th1 = run_single(3, small1, 0, SMALL1_HASH);
    let th2 = run_single(3, small2, 0, SMALL2_HASH);
    // these calls re-initialize the threads and put them on the 
    // run queue so they can be rerun.
    eqx_refork(th1);
    eqx_refork(th2);
    // run all threads till run queue empty.
    eqx_run_threads();
    trace("easy no-stack passed\n");

    // do a bunch of other ones
    let th_nop1 = run_single(3, nop_1, 0, NOP1_HASH);
    let th_mov_ident = run_single(3, mov_ident, 0, MOV_IDENT_HASH);
    let th_nop10 = run_single(3, nop_10, 0, NOP10_HASH);

    // now run all three 
    eqx_refork(th_nop1);
    eqx_refork(th_nop10);
    eqx_refork(th_mov_ident);
    eqx_run_threads();

    trace("second no-stack passed\n");

    // run all together: interleaving all 5 threads.
    eqx_refork(th1);
    eqx_refork(th2);
    eqx_refork(th_nop1);
    eqx_refork(th_nop10);
    eqx_refork(th_mov_ident);
    eqx_run_threads();

    trace("all no-stack passed\n");
}
