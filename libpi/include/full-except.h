#ifndef __EXCEPTION_FULL_REGS_H__
#define __EXCEPTION_FULL_REGS_H__
// exception handlers that load all registers [r0-r15 and spsr]
// can resume back.
#include "vector-base.h"

// for the register definition.
#include "switchto.h"

// can always pull <spsr> and <pc> from <regs> but they are 
// already around and this saves a cache miss.
//
// if we track the data internally, this gives the option of
// pushing multiple handlers cleanly.
typedef void (*full_except_t)(regs_t *r);
typedef int (*full_excepti_t)(regs_t *r);

// call this first: <override_p>=0: give an error if there is already
// a vector installed
void full_except_install(int override_p);

// get exception vectors.
static inline void * full_except_get_vec(void) {
    extern uint32_t full_except_ints[];
    return full_except_ints;
}

// call to set prefetch
full_except_t full_except_set_prefetch(full_except_t h);
// call to set data abort handler
full_except_t full_except_set_data_abort(full_except_t h);

// syscall: maybe give a nesting option for this one.
full_excepti_t full_except_set_syscall(full_excepti_t h);

#endif
