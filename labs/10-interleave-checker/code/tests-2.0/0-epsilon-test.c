// trivial check example: should not fail.  does nothing.
#include "check-interleave.h"

// do-nothing checker: should never fail.
static void eps_A(checker_t *c) { }

// completed.
static int eps_B(checker_t *c) { return 1; }

// initializes no state.
static void eps_init(checker_t *c) { }

// doesn't have anything to check: return 1 always.
static int  eps_check(checker_t *c) { return 1; }

checker_t eps_mk_checker(void) {
    return (struct checker) { 
        .state = 0,
        .A = eps_A,
        .B = eps_B,
        .init = eps_init,
        .check = eps_check
    };
}

void notmain(void) {
    // try with enable cache.
    enable_cache();

    // if you want to see what it's doing
    brk_verbose(0);
    struct checker c = eps_mk_checker();
    int n;
    if(!check(&c)) {
        assert(c.nerrors);
        panic("check should not have failed, trials=[%d], errors=%d!!\n", 
                c.ntrials, c.nerrors);
    } else {
        assert(!c.nerrors);
        exit_success("worked!  ntrials=[%d]\n", c.ntrials);
    }
}
