#include "rpi.h"
#include "pi-random.h"
#include "prototypes.h"

//************************************************************
// final routine: used to get <sp> and <lr> from arbitrary
// modes.   
//   0. record the original mode in a caller reg.
//   1. switch to the <mode> using:
//      msr cpsr_c, <mode reg>
//   2. get the SP register and write into the memory pointed
//      to by <sp> 
//   3. get the LR register and write into the memory pointed
//      to by <lr> 
//   4. switch back to the original mode.
//
// as an extension you can make a version that returns a 
// two element structure --- measure if faster!
void mode_get_lr_sp_asm(uint32_t mode, uint32_t *sp, uint32_t *lr);

static inline void
mode_get_lr_sp(uint32_t mode, uint32_t *sp, uint32_t *lr) {
    if(mode == USER_MODE)
        mode_get_lr_sp(SYS_MODE, sp, lr);
    else
        mode_get_lr_sp(mode, sp, lr);
}


void notmain(void) {
    // known garbage values we pick for <SP> and <LR>
    enum { SP = 0xfeedface, LR = 0xdeadbeef};
    uint32_t sp_set = SP, lr_set = LR;
    uint32_t sp_get = 0, lr_get = 0;

    trace("-----------------------------------------------------------\n");
    trace("part 4: now write and check mode_get_lr_sp_asm\n");
    uint32_t sp=0,lr=0;
    
    // from part 1 above
    cps_user_sp_set(SP);
    cps_user_lr_set(LR);
    assert(cps_user_sp_get() == SP);
    assert(cps_user_lr_get() == LR);

    // can pass any mode, but we know the answer for this one.
    // XXX: should add some more tests.
    mode_get_lr_sp_asm(SYS_MODE, &sp, &lr);
    trace("sp=%x, lr=%x\n", sp, lr);
    assert(lr == LR);
    assert(sp == SP);

    // make sure nothing weird happened.
    assert(cps_user_sp_get() == SP);
    assert(cps_user_lr_get() == LR);

    output("SUCCESS: all tests passed.\n");
    trace("-----------------------------------------------------------\n");
}
