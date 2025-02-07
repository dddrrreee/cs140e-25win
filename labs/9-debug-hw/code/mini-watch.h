#ifndef __MINI_WATCH_H__
#define __MINI_WATCH_H__
#include "armv6-debug-impl.h"
#include "full-except.h"

// structure describing a watchpoint: makes it easy
// to add other information without messing with
// client code.
typedef struct {
    uint32_t    fault_pc;       // pc fault happened at.
    void        *fault_addr;    // actual fault address
    unsigned    is_load_p;      // was it a load?
    regs_t      *regs;          // full set of registers at fault.
                                // *client can modify these* and
                                // the code continue execution using them.
} watch_fault_t;

static inline watch_fault_t watch_fault_mk(
    uint32_t fault_pc,
    void *fault_addr,
    unsigned is_load_p,
    regs_t *regs) 
{
    return (watch_fault_t) { 
        .fault_pc = fault_pc, 
        .fault_addr = fault_addr,
        .is_load_p = is_load_p,
        .regs = regs
    };
}

// watchpoint handler: call client routine with
// the data they gave to <mini_watch_init> and a
// structure describing the current fault.
typedef void (*watch_handler_t)(void *data, watch_fault_t *fault);

// initialize watchpoint: any fault will call 
// <h> passing <data> and a structure with info about
// current fault.
void mini_watch_init(watch_handler_t h, void *data);

// watch address <addr>
//
// could also give an option to pass a new handler routine.
void mini_watch_addr(void *addr);

// disable watchpoint on <addr>
void mini_watch_disable(void *addr);

// are we watching anything at all?
int mini_watch_enabled(void);

// called from exception handler: if the current 
// fault is a watchpoint, return 1
int mini_watch_is_fault(void);

#endif
