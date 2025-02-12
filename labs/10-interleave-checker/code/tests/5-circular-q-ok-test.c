// trivial check example: push and pop values without having a lock.
// should work fine
#include "check-interleave.h"
#include "libc/circular.h"

enum { N = 8 };
static int expected_A, expected_B;
static cq_t cq;

static void circq_A(checker_t *c) { 
    for(int i = 0; i < N; i++) {
        if(!cq_push(&cq, i))
            panic("could not push??\n");
        expected_A += i;
    }
}

static int circq_B(checker_t *c) { 
    // because we do not have a wait/yield construct,
    // we have to check that there are enough elements
    // in the queue before starting.
    //
    // this is a major limitation since we never do 
    // anything until there are enough elements (so 
    // don't check code correctness in the paths before
    // this point).  you should add a yield() routine.
    if(cq_nelem(&cq) < N) 
        return 0;

    for(int i = 0; i < N; i++) {
        cqe_t e = cq_pop(&cq);
        if(e != i)
            panic("invalid element=%d, expected=%d\n", e,i);
        expected_B += e;
    }
    assert(!cq_nelem(&cq));
    return 1;
}

static void circq_init(checker_t *c) { 
    cq_init(&cq,0); 
    expected_A = expected_B = 0;
}

// what is the failure case?
static int  circq_check(checker_t *c) { 
    assert(expected_A == expected_B);
    return (expected_A == expected_B);
}

checker_t circq_mk_checker(void) {
    return (struct checker) { 
        .state = 0,
        .A = circq_A,
        .B = circq_B,
        .init = circq_init,
        .check = circq_check
    };
}

void notmain(void) {
    // try with enable cache, without, and with different 
    // optimization levels.  [in general for all versions]
    enable_cache();

    struct checker c = circq_mk_checker();
    int n;
    if(!check(&c)) {
        assert(c.nerrors);
        panic("check should not have failed, trials=%d, errors=%d!!\n",
                c.ntrials, c.nerrors);
    } else {
        assert(!c.nerrors);
        exit_success("worked!  ntrials=[%d]\n", c.ntrials);
    }
}
