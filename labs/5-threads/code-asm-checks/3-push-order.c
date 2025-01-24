// When you `push` multiple registers, what is the order 
// they are written out? (Or, equivalently: where is 
// each one placed?)
#include "rpi.h"

enum { val1 = 0xdeadbeef, val2 = 0xFAF0FAF0 };

// you write <push_two> in <asm-check.S>
//
// should take a few lines:
//  use the "push" instruction to push <val1>  and <val2>
//  onto the memory pointed to by <addr>.
//
// returns the final address of <addr>
uint32_t *push_two(uint32_t *addr, uint32_t val1, uint32_t val2);

void notmain() {
    uint32_t v[4] = { 1, 2, 3, 4 };

    uint32_t *res = push_two(&v[2], val1, val2);
    assert(res == &v[0]);

    // note this also shows you the order of writes.
    if(v[2] == val2 && v[1] == val1) {
        assert(v[3] == 4);
        assert(v[0] == 1);
        todo("what does this imply?\n");
    } else if(v[1] == val2 && v[0] == val1) {
        assert(v[3] == 4);
        assert(v[2] == 3);
        todo("what does this imply?\n");
    } else 
        panic("unexpected result\n");
}
