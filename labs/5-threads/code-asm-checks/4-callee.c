// NOTE: you need to implement <todo> and handle all 
// registers from r0-r12
//
// we does some sleazy tricks to mechanically (automatically)
// verify which registers r0-r12 gcc thinks are caller-saved 
// and which are callee-saved.
//
// intuition:
//    we know from the <notes/caller-callee/README.md> that
//    if a routine contains a single gcc's inline assembly 
//    statement that "clobbers" a register (i.e., tells gcc 
//    that the assembly modifies it) that gcc will save and 
//    restore the register iff it's callee-saved and do 
//    nothing (just return) if it's caller-saved.
//
#include "rpi.h"

// write <is_empty>: given a routine address <fp>, return 1 if 
// <fp> does nothing but return.
//  1. figure out what machine instruction should be the first
//     (only) instruction in the routine.
//  2. figure out how to get the first instruction given
//     a function address.
//  3. return 1 if (2) == (1).
//
// we use this to tell if a register <r> is caller saved:
//   1. generate a routine that only clobbers <r>
//   2. if routine is empty, <r> was a caller reg.
//   3. if routine is not empty, <r> was a callee reg.
static inline int is_empty(void (*fp)(void)) {
    todo("returns 1 if routine does nothing besides return");
}

// note: you can do these better with macros.
static void clobber_r0(void) {
    asm volatile("" : : : "r0");
}
static void clobber_r1(void) {
    asm volatile("" : : : "r1");
}
// ... do the rest up to r12 ....


// FILL this in with all caller-saved registers.
// these are all the registers you *DO NOT* save during 
// voluntary context switching 
//
// NOTE: we know we have to save r13,r14,r15 so ignore
// them.
void check_cswitch_ignore_regs(void) {
    assert(is_empty(clobber_r0));
    assert(is_empty(clobber_r1));
    todo("add all your non-saved registers here");

    // if you reach here it passed.
    trace("ignore regs passed\n");
}


// put all the regs you *do* save during context switching
// here [callee saved] 
//
// NOTE: we know we have to save r13,r14,r15 so ignore
// them.
void check_cswitch_save_regs(void) {
    todo("add all your saved registers here");
    trace("saved regs passed\n");
}

void notmain() {
    check_cswitch_ignore_regs();
    check_cswitch_save_regs();
    trace("SUCCESS\n");
}
