
// what to do: search below for:
//  - part 1
//  - 

// some simple tests to make really sure that USER banked registers and
// instructions we need for switching USER level processes work
// as expected:
//   - ldm ^ (with one and multiple registers)
//   - stm ^ (with one and multiple registers)
//   - cps to SYSTEM and back.
//
// basic idea: write known values, read back and make sure they are 
// as expected.
//
// [should rewrite so that this uses randomized values for some number of
// times.]
#include "rpi.h"
#include "pi-random.h"
#include "prototypes.h"


//*******************************************************
// routines to get and set user <sp> and <lr> registers
// using the cps instruction and switching to SYSTEM.

// get USER <sp> by using the <cps> instruction to switch 
// to SYSTEM mode and back.  
uint32_t cps_user_sp_get(void);
void cps_user_sp_set(uint32_t sp);

// get USER <lr> by using <cps> to switch to SYSTEM
// and back.
uint32_t cps_user_lr_get(void);
void cps_user_lr_set(uint32_t lr);

// get both at once with a single call.
uint32_t cps_user_sp_lr_get(uint32_t sp_lr[2]);
void cps_user_sp_lr_set(uint32_t sp_lr[2]);

void notmain(void) {
    // both <sp> and <lr> values are just whatever garbage
    // is there on bootup.  i'm honestly a bit surprised 
    // it's not <0> (at least on my r/pi).
    output("uninitialized user sp=%x\n", cps_user_sp_get());
    output("uninitialized user lr=%x\n", cps_user_lr_get());
    
    /*********************************************************
     * part 1: _set/_get USER sp,lr using the <cps> method 
     *
     * we give you <user_sp_get>.  you'll write:
     *  - user_sp_set
     *  - user_lr_set
     *  - user_lr_get
     * in <0-user-sp-lr-asm.S>:
     */

    trace("-----------------------------------------------------------\n");
    trace("part 1: get/set USER sp/lr by switching to SYSTEM and back\n");

    // known garbage values we pick for <SP> and <LR>
    enum { SP = 0xfeedface, LR = 0xdeadbeef};
    uint32_t sp_set = SP, lr_set = LR;
    // set the registers to traditional OS weird values.
    // [you write both of these]
    cps_user_sp_set(sp_set);  // after: user <sp> should be == SP
    cps_user_lr_set(lr_set);  // after: user <lr> should be == LR

    // make sure what we *_set is what we *_get()
    uint32_t sp_get = cps_user_sp_get();
    uint32_t lr_get = cps_user_lr_get();    // you write this.
    trace("\tgot USER sp=%x\n", sp_get);
    trace("\tgot USER lr=%x\n", lr_get);
    assert(sp_get == SP);
    assert(lr_get == LR);


    enum { N = 20 };
    trace("about to do [%d] random runs\n", N);
    for(int i = 0; i < N; i++) {
        uint32_t sp = pi_random();
        uint32_t lr = pi_random();

        cps_user_sp_set(sp);  
        cps_user_lr_set(lr);  

        assert(sp == cps_user_sp_get());
        assert(lr == cps_user_lr_get());
    }
    trace("success for [%d] random runs!\n", N);

    // reset USER sp/lr to something else so we can do another
    // test.
    cps_user_sp_set(0);
    cps_user_lr_set(0);
    assert(cps_user_sp_get() == 0);
    assert(cps_user_lr_get() == 0);

    trace("SUCCESS: part 1 passed\n");
    trace("-----------------------------------------------------------\n");
}
