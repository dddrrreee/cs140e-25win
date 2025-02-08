#ifndef __SWITCHTO_H__
#define __SWITCHTO_H__
#include "cpsr-util.h"

// offsets used for registers in <regs> array.
// exception trampoline should save in these
// locations.
enum {
    REGS_SP = 13,
    REGS_LR = 14,
    REGS_PC = 15,
    REGS_CPSR = 16
};

// in future: might need process pointer as well.
typedef struct {
    // all 16 registers and cpsr
    uint32_t regs[17];
} regs_t;

// get <sp> and <lr> for mode <mode>.
//   raw, no checking: only works for privileged mode.
void priv_get_sp_lr_asm(uint32_t mode, uint32_t *sp, uint32_t *lr);

// defined in staff-exception-asm.S: get the <sp> and <lr>
// from the mode in <r>
static inline void mode_get_sp_lr(regs_t *r) {
    uint32_t mode = r->regs[REGS_CPSR];

    // trivial to get this.
    if(mode_get(mode) == USER_MODE)
        panic("cannot handle user level\n");

    priv_get_sp_lr_asm(mode, 
        &r->regs[REGS_SP],
        &r->regs[REGS_LR]);
}

// swiich to privileged mode using <r>
void switchto_priv_asm(regs_t *r)  __attribute__((noreturn));
// swiich to user mode using <r>
void switchto_user_asm(regs_t *r)  __attribute__((noreturn));

// figure out if we are jumping to user or
// a privileged mode and use the right 
// context switching routine.
static inline void __attribute__((noreturn)) 
switchto(regs_t *r)  
{
    if(mode_get(r->regs[REGS_CPSR]) == USER_MODE) {
        switchto_user_asm(r);
    } else {
        switchto_priv_asm(r);
    }
}

// save full register set (with cpsr) into <regs> and call
//  fn(regs,arg) after.
// you can switchto <regs>
int switchto_save_regs(void (*fn)(uint32_t *regs, void *arg), 
                    uint32_t *regs, void *arg);


typedef void (*switch_fn_t)(regs_t *) __attribute__((noreturn));

// cswitch, saving current registers in <old> and 
// loading the registers from <next>, jump to <fn>
// when done.
void cswitch_asm(regs_t *old, regs_t *next, switch_fn_t fn);

// perhaps we should have a stack stealing snapshot?
static inline void 
switchto_cswitch(regs_t *old, regs_t *next) {
    assert(mode_get(cpsr_get()) != USER_MODE);

    // could hardire into
    if(mode_get(next->regs[REGS_CPSR]) == USER_MODE) {
        cswitch_asm(old, next, switchto_user_asm);
    } else {
        cswitch_asm(old, next, switchto_priv_asm);
    }
}

// setup switchto registers to call <fn> 
// at mode level <mode>.
// stack pointer <sp>
// and if the code returns jump to <on_exit>
//
// casting all the function pointers is a real pain, so
// idk what to do about it.
//
static inline regs_t 
switchto_mk(
    uint32_t fn, 
    void *sp,
    uint32_t cpsr,
    void (*on_exit)(int retv)) 
{
    return (regs_t) { 
        .regs[REGS_PC] = fn,
        .regs[REGS_SP] = (uint32_t)sp,
        .regs[REGS_LR] = (uint32_t)on_exit,
        .regs[REGS_CPSR] = cpsr
    };
}

// return the original level?
uint32_t switchto_user(void);

#endif
