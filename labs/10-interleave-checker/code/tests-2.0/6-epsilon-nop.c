// trivial check example: should *always* fail
#include "check-interleave.h"

// do-nothing checker: should never fail.
static void eps_A(checker_t *c) { 
    volatile int x;
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    x;
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    x;
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    x;
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
}
static int eps_B(checker_t *c) { 
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    asm volatile("nop");
    return 1; 
}
static void eps_init(checker_t *c) { }
static int  eps_check(checker_t *c) { 
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
        output("done checking: trials=[%d] errors=[%d]!!\n", 
                c.ntrials, c.nerrors);
        exit_success("succeeded as expected!\n");
    } else {
        panic("should not fail\n");
    }
}
