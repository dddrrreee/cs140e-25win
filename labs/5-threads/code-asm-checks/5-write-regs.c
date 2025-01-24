// double check: that your context switch code saves values
// to the expected known locations on the stack.
//
// write assembly routine 
//  void write_regs_to_stack(uint32_t **store_sp);
// to 
//   1. initialize the callee-saved regs and lr to known
//      values (r4=4, r5=5, ... lr = 14)
//   2. push (save) them to the stack.
//   3. call <after_writing_regs> to check they are written
//      where expected.
#include "rpi.h"

// #include "rpi-thread.h"

// implement in <asm-checks.S>
void write_regs_to_stack(uint32_t **store_sp);

// hack so <write_regs_to_stack> stores sp in a way that we
// can check.
static uint32_t *stored_sp = 0;

// stack offsets we expect.  
enum { 
    // callee
    R4_OFFSET = 0,
    R5_OFFSET,
    R6_OFFSET,
    R7_OFFSET,
    R8_OFFSET,
    R9_OFFSET,
    R10_OFFSET,
    R11_OFFSET,

    R14_OFFSET = 8,     // these are the same.
    LR_OFFSET = 8
};

// <sp> = current value of the stack pointer.
//    is set to where the last register was saved.
void after_writing_regs(uint32_t *sp) {
    printk("sp=%p, *stored_sp=%p\n", sp, stored_sp);

    // check that you store the sp to a pointer correctly.
    assert(stored_sp == sp);

    // use this to check that your offsets are correct.
    for(unsigned i = 0; i < 9; i++) {
        unsigned r = i == 8 ? 14 : i + 4;
        printk("sp[%d]=%d, expect=%d\n", i, sp[i],r);
        assert(sp[i] == r);
    }

    trace("sp[%d]=%d, expect=%d\n", R4_OFFSET, sp[R4_OFFSET],4);
    assert(sp[R4_OFFSET] == 4);

    trace("sp[%d]=%d, expect=%d\n", R5_OFFSET, sp[R5_OFFSET],5);
    assert(sp[R5_OFFSET] == 5);

    trace("sp[%d]=%d, expect=%d\n", R6_OFFSET, sp[R6_OFFSET],6);
    assert(sp[R6_OFFSET] == 6);

    trace("sp[%d]=%d, expect=%d\n", R7_OFFSET, sp[R7_OFFSET],7);
    assert(sp[R7_OFFSET] == 7);

    trace("sp[%d]=%d, expect=%d\n", R8_OFFSET, sp[R8_OFFSET],8);
    assert(sp[R8_OFFSET] == 8);

    trace("sp[%d]=%d, expect=%d\n", R9_OFFSET, sp[R9_OFFSET],9);
    assert(sp[R9_OFFSET] == 9);

    trace("sp[%d]=%d, expect=%d\n", R10_OFFSET, sp[R10_OFFSET],10);
    assert(sp[R10_OFFSET] == 10);

    trace("sp[%d]=%d, expect=%d\n", R11_OFFSET, sp[R11_OFFSET],11);
    assert(sp[R11_OFFSET] == 11);

    trace("sp[%d]=%d, expect=%d\n", LR_OFFSET, sp[LR_OFFSET],14);
    assert(sp[LR_OFFSET] == 14);

    // can't return b/c callee regs got trashed in assembly.
    clean_reboot();
}

void notmain() {
    trace("about to write known register values to the stack\n");
    write_regs_to_stack(&stored_sp);
    not_reached();
}
