// a simple example of how the RFE instruction works.
// see: <1-rfe-asm.S> for the associated assembly 
// routines.
#include "rpi.h"
#include "asm-helpers.h"
#include "cpsr-util.h"

// takes a pointer to a 2-entry array holding
// the registers you want to resume to.
// see <1-rfe-asm.S>
void rfe_asm(uint32_t regs[2]);

// in <1-rfe-asm.S>: loads sp and calls <rfe_c_code>
void rfe_trampoline_asm(void);

// called by <rfe_trampoline_asm>.
//   [can extend: try to go to each different mode.]
//
// prints out the stack pointer passed in and checks that it's
// indeed running at USER_MODE.
void rfe_c_code(uint32_t sp) {
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
    /***********************************************************
     * check rfe works as expected and simultaneously
     * loads both the pc and cpsr by doing a do a simple 
     * <rfe> that will jump to <rfe_trampoline_asm>
     * with <cpsr>=USER_MODE (see: <1-srs-rfe-asm.S>)
     */

    // rfe takes a pointer to a 2-entry array.
    uint32_t regs[2];
    // 1. set the pc entry in the array to <rfe_trampoline_asm>.
    //    this assembly routine will load a stack pointer and 
    //    then call <rfe_c_code> (above)
    regs[0] = (uint32_t)rfe_trampoline_asm;   // in <1-srs-rfe-asm.S>
    // 2 set the spsr entry in the array to <USER_MODE>
    regs[1] = USER_MODE;
    trace("about to jump to pc=[%x] with cpsr=%x\n",
            regs[0], regs[1]);
    // expected result: should see print values in <rfe_c_code>
    rfe_asm(regs);
    not_reached();
}
