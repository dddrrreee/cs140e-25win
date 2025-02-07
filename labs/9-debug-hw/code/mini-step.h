// very simple version of single stepping.
#ifndef __MINI_WATCH_H__
#define __MINI_WATCH_H__

// defines <regs_t>
#include "switchto.h"

//*******************************************************
// these four routines can be used in isolation.  you
// would have to check for a mismatch fault in the 
// exception handler.

// turn mismatching (single-step) on. 
void mismatch_on(void);

// turn mismatch (single-step) off.
void mismatch_off(void);

// set the pc to run, mismatch everything else.
// when you first start, set it to 0 (or any other
// address that doesn't get executed):
//     mismatch_pc_set(0);
uint32_t mismatch_pc_set(uint32_t pc);

// set a mismatch fault on the pc register in <r>
// context switch to <r>
void mismatch_run(regs_t *r);

//*******************************************************
// the following routines build on the primitives above,
// but do their own exception handling so are less flexible.

typedef struct {
    uint32_t    fault_pc;      // pc faulted at.
    regs_t      *regs;           // full set of registers.
} step_fault_t;

static inline step_fault_t 
step_fault_mk(
    uint32_t fault_pc,
    regs_t *regs) 
{
    return (step_fault_t) { 
        .fault_pc = fault_pc, 
        .regs = regs
    };
}

// step handler registered by the client.
//   - <data> is provided by the client and passed on each
//    handler invocation.
//   - <fault> describes the fault location.
typedef void (*step_handler_t)(void *data, step_fault_t *fault);

// one time initialize.  setup to call <h> with <data> on
// each fault.
void mini_step_init(step_handler_t h, void *data);

// run <fn> in single step mode with <arg> 
uint32_t mini_step_run(void (*fn)(void*), void *arg);
#endif
