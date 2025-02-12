#ifndef __SWITCHTO_H__
#define __SWITCHTO_H__
// some changes for lab 14

// wrappers for switchto and cswitch routines: checks if the
// destination is user level or privileged so can call the 
// right version.
#include "cpsr-util.h"

// offsets used for registers in <regs_t> array.
// interrupt/exception trampoline should save 
// the registers in these locations.
enum {
    REGS_SP = 13,
    REGS_LR = 14,
    REGS_PC = 15,
    REGS_CPSR = 16,
    REGS_R0 = 0,
    REGS_R1 = 1,
    REGS_R2 = 2,
};

// we keep things in a struct for some partial type checking.
typedef struct {
    // all 16 general purpose registers + cpsr
    uint32_t regs[17];
} regs_t;
_Static_assert(sizeof(regs_t) == 4*17, "<regs_t> is wrong size");

// get <sp> and <lr> for mode <mode>.
//   raw, no checking: only works for privileged mode.
void priv_get_sp_lr_asm(uint32_t mode, uint32_t *sp, uint32_t *lr);

// implemented in <staff-exception-asm.S>: given the saved
// registers <r>, pull out the mode from (r->regs[16]) and 
// store that mode's <sp> and <lr> into <r> at the right 
// offsets in <r>
//
// we use this to patch up registers after saving them assuming
// we came from user mode.   alternative approach: 
// swap the exception handlers when doing a <switchto> based on 
// where jumping to.   this is faster but more fragile since every 
// location that switches the <cpsr> better update the exception 
// vector.  
static inline void mode_get_sp_lr(regs_t *r) {
    uint32_t mode = r->regs[REGS_CPSR];

    // the save code should have handled this;
    // so we don't need.
    if(mode_get(mode) == USER_MODE)
        panic("should not be trying to patch up user mode\n");

    // NOTE: lab 13 had the name swapped.
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
// call the appropriate <switchto> routine on <new>
//
// this is a voluntary cswitch so it doesn't matter
// if the current mode is priv or user.  it only matters 
// where we are going.  (though obviously we would 
// fault if we try to switch from user to priv)
void cswitchto_user_asm(regs_t *old, regs_t *next);
void cswitchto_priv_asm(regs_t *old, regs_t *next);

// perhaps we should have a stack stealing snapshot?
static inline void 
switchto_cswitch(regs_t *old, regs_t *next) {
    assert(mode_get(cpsr_get()) != USER_MODE);

    // could hardire into
    if(mode_get(next->regs[REGS_CPSR]) == USER_MODE) {
        cswitchto_user_asm(old, next);
    } else {
        panic("switching to privilege\n");
        cswitchto_priv_asm(old, next);
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
    uint32_t on_exit)  // void (*on_exit)(int retv)) 
{
    return (regs_t) { 
        .regs[REGS_PC] = fn,
        .regs[REGS_SP] = (uint32_t)sp,
        .regs[REGS_LR] = on_exit,
        .regs[REGS_CPSR] = cpsr
    };
}

// return the original level?
uint32_t switchto_user(void);

#endif
