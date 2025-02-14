// some simple tests to make really sure that USER banked registers and
// instructions we need for switching USER level processes work
// as expected:
//   - ldm ^ (with one and multiple registers)
//   - stm ^ (with one and multiple registers)
//
// basic idea: write known values, read back and make sure they are 
// as expected.
#include "rpi.h"
#include "pi-random.h"
#include "prototypes.h"

//*******************************************************
// routines to get and set user <sp> and <lr> registers
// using the ldm/stm with the carat "^" operator.  should
// be faster than switching modes.

// use ldm/stm with the ^ modifier to get USER mode <sp>
void mem_user_sp_get(uint32_t *sp);
void mem_user_sp_set(const uint32_t *sp);

// use ldm/stm with the ^ modifier to get USER mode <lr>
void mem_user_lr_get(uint32_t *sp);
void mem_user_lr_set(const uint32_t *sp);

// use ldm/stm with the ^ modifier to get USER 
// mode <lr> and <sp>
void mem_user_sp_lr_get(uint32_t sp_lr[2]);
void mem_user_sp_lr_set(const uint32_t sp_lr[2]);


void notmain(void) {
    /*********************************************************
     * part 2: set and get USER sp,lr using the ldm/stm versions.
     * you'll write:
     *   - <mem_user_sp_get>
     *   - <mem_user_sp_set>
     *   - <mem_user_lr_get>
     *   - <mem_user_lr_set>
     */
    trace("-----------------------------------------------------------\n");
    trace("part 2: set/get USER sp/lr using ldm/stm with ^\n");
    enum { SP = 0xfeedface, LR = 0xdeadbeef};
    uint32_t sp_set = SP, lr_set = LR;
    uint32_t sp_get = 0, lr_get = 0;

    // set them back to known values.
    sp_set = SP;
    lr_set = LR;
    mem_user_sp_set(&sp_set);       
    mem_user_lr_set(&lr_set);
    // read back 
    mem_user_sp_get(&sp_get);       
    mem_user_lr_get(&lr_get);
    trace("\tgot USER sp=%x\n", sp_get);
    trace("\tgot USER lr=%x\n", lr_get);
    assert(sp_get == SP);
    assert(lr_get == LR);

    // check previous operations still work, too, twice.
    assert(sp_get == cps_user_sp_get());
    assert(lr_get == cps_user_lr_get());
    assert(sp_get == cps_user_sp_get());  // nothing should change
    assert(lr_get == cps_user_lr_get());

    enum { N = 20 };
    trace("about to do [%d] random runs\n", N);
    for(int i = 0; i < N; i++) {
        uint32_t sp = sp_set = pi_random();
        uint32_t lr = lr_set = pi_random();

        mem_user_sp_set(&sp_set);  
        mem_user_lr_set(&lr_set);  

        mem_user_sp_get(&sp_get);  
        mem_user_lr_get(&lr_get);  

        assert(sp == sp_get);
        assert(lr == lr_get);
    }
    trace("success for [%d] random runs!\n", N);

    // reset USER sp/lr to something else so we can do another
    // test.
    cps_user_sp_set(0);
    cps_user_lr_set(0);
    assert(cps_user_sp_get() == 0);
    assert(cps_user_lr_get() == 0);

    trace("SUCCESS: part 2 passed\n");
    trace("-----------------------------------------------------------\n");


    /*********************************************************
     * part 3: _set/_get USER sp,lr using the double
     * memory ldm/stm versions.
     */
    trace("-----------------------------------------------------------\n");
    trace("part 3: set/get USER sp/lr using single ldm/stm ^ instruction\n");
    const uint32_t splr_set[2] = { SP, LR };
    uint32_t splr_get[2];
    mem_user_sp_lr_set(splr_set);
    mem_user_sp_lr_get(splr_get);
    trace("\tgot USER sp=%x\n", splr_get[0]);
    trace("\tgot USER lr=%x\n", splr_get[1]);
    assert(SP == splr_get[0]);
    assert(LR == splr_get[1]);

    // double check we get the same thing on a second invocation.
    splr_get[0] = splr_get[1] = 0;
    mem_user_sp_lr_get(splr_get);
    assert(SP == splr_get[0]);
    assert(LR == splr_get[1]);

    trace("part 3: passed\n");

    trace("-----------------------------------------------------------\n");

}
