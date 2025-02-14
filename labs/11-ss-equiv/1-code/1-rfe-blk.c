// small step for easy debugging: change <1-rfe-example.c> 
// to take a 17-entry array with the full set of registers 
// and update the offset calculations.
#include "rpi.h"
#include "asm-helpers.h"
#include "cpsr-util.h"

// change <rfe_asm> to take a block of 17 uint32's.
//  - is in <1-rfe-asm.S>
void blk_rfe_asm(uint32_t regs[17]);

// you write this (in <1-rfe-asm.S>) 
//   loads a1,a2,a3, sp and calls <rfe_c_code>
void blk_trampoline_asm(void);

// called by <switch_trampoline_asm>.
//   [can extend: try to go to each different mode.]
//
// prints out the stack pointer passed in and checks that it's
// indeed running at USER_MODE.
void rfe_c_code(uint32_t sp) 
{
    trace("sp=[%x], should equal INT_STACK_ADDR=%x\n", 
            sp, INT_STACK_ADDR);
    assert(sp == INT_STACK_ADDR);

    uint32_t cpsr = cpsr_get();

    // NOTE: carry flags could be different for different compilers.
    cpsr = cpsr_clear_carry(cpsr);

    trace("cpsr=%x, mode=%s\n", cpsr, mode_str(cpsr));
    if(mode_get(cpsr) != USER_MODE)
        panic("wrong mode!\n");
    else
        trace("SUCCESS: we are at user level!\n");

    clean_reboot();
}

void notmain(void) {
    // make <regs> large enough (17) to describe the entire 
    // process context: 16 general registers + the cpsr.
    uint32_t regs[17];

    // 1. as before: set the pc entry (r15) in the array 
    //    to <rfe_trampoline_asm>.
    regs[15] = (uint32_t)blk_trampoline_asm;   // in <1-srs-rfe-asm.S>
    // 2 as before: set the cpsr entry (16) <USER_MODE>
    regs[16] = USER_MODE;

    trace("about to jump to pc=[%x] with cpsr=%x\n",
            regs[15], regs[16]);

    // expected result: should see print values in <rfe_c_code>
    blk_rfe_asm(regs);
    not_reached();
}
