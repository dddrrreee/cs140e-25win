// another small step for easy debugging: load sp from 
// the given block.
#include "rpi.h"
#include "asm-helpers.h"
#include "cpsr-util.h"

// change <rfe_asm> to take a block of 17 uint32's.
void switchto_user_asm(uint32_t regs[17]);

// new: called indirectly by <sp_rfe_asm>
static void load_sp_code(uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3) 
{
    // make it easier by deriving stack address from
    // register.
    enum { max_diff = 128 };
    uint32_t got_sp =  (uint32_t)&got_sp;
    uint32_t exp_sp = INT_STACK_ADDR;
    int diff = exp_sp - got_sp;

    if(diff < 0)
        panic("stack = %x, expected around %x\n", got_sp, exp_sp);
    if(diff > max_diff)
        panic("stack = %x, expected within %d of %x\n", 
            got_sp, max_diff, exp_sp);

    trace("r0=%x, r1=%x, r2=%x, r3=%x sp=[%x]\n", r0,r1,r2,r3,got_sp);
    assert(r1 == r1);
    assert(r2 == r2);
    assert(r3 == r3);
    uint32_t cpsr = cpsr_get();
    if(mode_get(cpsr) != USER_MODE)
        panic("wrong mode!\n");

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

    // 1 NEW: load stack pointer directly
    regs[13] = INT_STACK_ADDR;

    // 2. NEW: set the pc entry (r15) in the array 
    //    to <load_sp_code>: <load_sp_rfe_asm> should handle 
    //    loading user sp.
    regs[15] = (uint32_t)load_sp_code; 

    // 3 OLD: as before: set the cpsr entry (16) <USER_MODE>
    regs[16] = USER_MODE;

    // 4: NEW: load the four argument regs.
    regs[0] = 0;
    regs[1] = 1;
    regs[2] = 2;
    regs[3] = 3;

#if 1
    trace("about to jump to pc=[%x] with cpsr=%x, sp=%x\n",
            regs[15], regs[16], regs[13]);
#else
    panic("about to jump to pc=[%x] with cpsr=%x\n",
            regs[15], regs[16]);
#endif

    // expected result: <load_sp_code> should pass
    switchto_user_asm(regs);
    not_reached();
}
