// trivial check example: should *always* fail
#include "check-interleave.h"

// do-nothing checker: should never fail.
static void eps_A(checker_t *c) { }
static int eps_B(checker_t *c) { return 1; }
static void eps_init(checker_t *c) { }
// if we are no longer doing sequential checking, always fail.
static int  eps_check(checker_t *c) { 
    if(c->interleaving_p)
        return 0; 
    return 1;
}

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

    brk_verbose(0);
    struct checker c = eps_mk_checker();
    int n;
    if(check(&c)) {
        assert(!c.nerrors);
        panic("check should have failed, trials=%d!!\n", 
                c.ntrials);
    } else {
        output("done checking: trials=[%d] errors=[%d]!!\n", 
                c.ntrials, c.nerrors);
        assert(c.nerrors == c.ntrials);
        exit_success("failed as expected!\n");
    }
}
