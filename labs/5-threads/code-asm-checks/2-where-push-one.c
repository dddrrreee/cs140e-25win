// Write assembly code to answer: 
//   does push change its address register before writing 
//   to it or after?
// You need this for context switching.
//
// NOTE: to save time we give you all the code below (maybe we
// shouldn't) --- don't just blindly make it pass, think about 
// what its doing and why.  We won't always be there with
// starter code :)
#include "rpi.h"

enum { val1 = 0xdeadbeef, val2 = 0xFAF0FAF0 };

// you write <push_one> in <asm-check.S>
//
// should take a few lines:
//  use the "push" instruction to push <val1>
//  onto the memory pointed to by <addr>.
//
// returns the final address
uint32_t *push_one(uint32_t *addr, uint32_t val1);

void notmain() {
    uint32_t v[4] = { 1, 2, 3, 4 };

    // use <push> to write an unlikely value <val1> using
    // a pointer <&v[2]>.
    // - if <v[2]> == <val1> then we know push stores 
    //   immediately, and modifies its pointer register after.
    // - if <v[1]> == <val1> then we know push modifies
    //   its pointer register first, before storing.
    uint32_t *res = push_one(&v[2], val1);
    assert(res == &v[1]);

    // note this also shows you the order of writes.
    if(v[2] == val1) {
        // make sure nothing else got changed.
        assert(v[3] == 4);
        assert(v[1] == 2);
        assert(v[0] == 1);
        trace("wrote value before modifying pointer\n");
    } else if(v[1] == val1) {
        // make sure nothing else got changed.
        assert(v[3] == 4);
        assert(v[2] == 3);
        assert(v[0] == 1);
        trace("wrote value after modifying pointer\n");
    } else 
        panic("unexpected result\n");
}
