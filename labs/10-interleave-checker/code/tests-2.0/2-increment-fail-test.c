// trivial check example: increments variable without having a lock.
// will fail.
#include "check-interleave.h"

// trivial counter.   broken.
static volatile int cnt = 0;
static void cnt_A(checker_t *c) { cnt++; } 
static int cnt_B(checker_t *c) { cnt++; return 1; }
static void cnt_init(checker_t *c) { cnt = 0; }
static int  cnt_check(checker_t *c) { return cnt == 2; }

checker_t cnt_mk_checker(void) {
    return (struct checker) { 
        .state = 0,
        .A = cnt_A,
        .B = cnt_B,
        .init = cnt_init,
        .check = cnt_check
    };
}

void notmain(void) {
    // try with enable cache.
    enable_cache();

    struct checker c = cnt_mk_checker();
    int n;
    if(check(&c))
        panic("check should have failed!\n");
    else 
        exit_success("check failed as it should have, ntrials=[%d], nerrors=[%d]\n", 
                c.ntrials, c.nerrors);
}
