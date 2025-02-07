// staff code: don't modify for today.

// simplistic setup so can:
//   (1) call prefetch and data abort with the complete register state needed
//     to switch to another process/another priviledged level.  (all 16 gp regs
//     and the spsr: not handling floating point)
//   (2) dynamically override the handlers for different exceptions.  previously
//      we used the same name to indicate which code would be called.  this
//      is faster (no load) but this makes it awkward to package code together 
//      b/c of duplicate symbol names.  (could just do self-modifying
//      code to patch the jump)
//
// limits:
//  - only handles data/prefetch abort (easy to change)
//  - only handles a single override: in reality may want a stack of these.
//  - pretty slow.  can do various tricks (JIT + customize and swap vectors).
//
// i think want to make it so things are pushed last in / first run.  that way
// you can setup the virtual memory with a handler of last resort and then 
// push other things on top of it that get to try first.   maybe have some
// way to tell it to stop calling subsequent handlers.  issue is that we don't
// know what is really going on.
//
// have them change so you can push multiple?   
// what exactly are the rules for demuxing multiple?  
//   1. e.g., if one does a mutation of the regs, do you pass 
//      them through?
//   2. i think you could "lock" switchto (or pass a different type?)
//      so that they have to get to the last handler.
//
//  for pix: i think just grab everything you need an put in the same
//  dir.  can then spend time sharding things out into different dirs.
// 
//  i guess keep this hack of putting into a byte array so they don't
//  get confused?  the downside is that it doesn't generalize into 
//  passing a ramfs or a set of options.

#include "rpi.h"
#include "vector-base.h"
#include "full-except.h"
#include "cpsr-util.h"

void mode_get_lr_sp_asm(uint32_t mode, uint32_t *sp, uint32_t *lr);

// this is gross: but i'm not sure how to get the 
static inline void fixup_regs(regs_t *r, uint32_t spsr) {
    // all the trampolines assume we come from user level.
    if(mode_get(spsr) == USER_MODE)
        return;

    // extra overhead but we ignore for the moment since everything
    // is pretty slow.  can always jit the original calls.
    priv_get_sp_lr_asm(spsr, &r->regs[13], &r->regs[14]);
}

static full_except_t prefetch_handler, data_abort_handler;
static full_excepti_t syscall_handler;

// forward the prefetch abort exception to client supplied handler.
//
// could have the assembly do this directly.
void prefetch_abort_full_except(regs_t *r, uint32_t spsr, uint32_t pc) {
    if(!prefetch_handler)
        panic("unhandled prefetch abort from pc=%x\n", pc);
    fixup_regs(r, spsr);
    prefetch_handler(r);
    panic("not reached: or could call switchto\n");
}

// forward the data abort exception to client supplied handler.
void data_abort_full_except(regs_t *r, uint32_t spsr, uint32_t pc) {
    if(!data_abort_handler)
        panic("unhandled data abort from pc=%x\n", pc);

    // we should swap the handlers depending on where we are jumping to.
    fixup_regs(r, spsr);
    data_abort_handler(r);
    panic("not reached: or could call switchto\n");
}

int syscall_full_except(regs_t *r, uint32_t spsr, uint32_t pc) {
    if(mode_get(spsr) != USER_MODE)
        panic("only handling system calls from user mode.\n");

    // should check these in general.  also that its near the
    // interrupt stack address.
    uint32_t sp;
    demand((uint32_t)&sp > 100000, illegal stack pointer!);

    // if we are at SUPER level would just need to subtract 68 from
    // sp, but getting the original would be a pain.
    if(!syscall_handler)
        panic("unhandled syscall from pc=%x\n", pc);

    return syscall_handler(r);
}

// set client exception handler.
//
// overrides the existing one: could extend to push them 
// in a stack (so can do multiple tools).  would swap the
// vector in vector base reg
full_except_t full_except_set_prefetch(full_except_t h) {
    full_except_t o = prefetch_handler;
    prefetch_handler = h;
    return o;
}

full_except_t full_except_set_data_abort(full_except_t h) {
    full_except_t o = data_abort_handler; 
    data_abort_handler = h;
    return o;
}

// could rewrite the trampoline for more speed.
full_excepti_t full_except_set_syscall(full_excepti_t h) {
    full_excepti_t o = syscall_handler; 
    syscall_handler = h;
    return o;
}

// install the default exception vectors: client can
// have their own as long as they call the full except
// handlers above.
//
// <override_p>: overwrite whatever is there.
void full_except_install(int override_p) {
    extern uint32_t full_except_ints[];
    void *v = full_except_ints;

    if(override_p) 
        vector_base_set(v);
    else {
        void *addr = vector_base_get();
        if(!addr)
            vector_base_set(v);
        else if(addr != v)
            panic("already have exception handlers installed: addr=%x\n", addr);
    }
}
