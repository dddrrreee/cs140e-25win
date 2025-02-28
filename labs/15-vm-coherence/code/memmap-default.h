#ifndef __MEMMAP_DEFAULT_H__
#define __MEMMAP_DEFAULT_H__

// the symbols 
#include "memmap.h"

// we put all the default address space enums here:
// - kernel domain
// - user domain
// - layout.

#define MB(x) ((x)*1024*1024)

// default domains for kernel and user.  not required.
enum {
    dom_kern = 1, // domain id for kernel
    dom_user = 2,  // domain id for user

    // setting for the domain reg: 
    //  - client = checks permissions.
    //  - each domain is 2 bits
    dom_bits = DOM_client << (dom_kern*2) 
        | DOM_client << (dom_user*2) 
};

enum { 
    // default no user access permissions
    no_user = perm_rw_priv,
    // default user access permissions
    user_access = perm_rw_user,

};

// the default asid we use.
enum {
    default_ASID = 1
};


// These are the default segments (segment = one MB)
// that need to be mapped for our binaries so far
// this quarter. 
//
// these will hold for all our tests today.
//
// if we just map these segments we will get faults
// for stray pointer read/writes outside of this region.
enum {
    // code starts at 0x8000, so map the first MB
    //
    // if you look in <libpi/memmap> you can see
    // that all the data is there as well, and we have
    // small binaries, so this will cover data as well.
    //
    // NOTE: obviously it would be better to not have 0 (null) 
    // mapped, but our code starts at 0x8000 and we are using
    // 1mb sections (which require 1MB alignment) so we don't
    // have a choice unless we do some modifications.  
    //
    // you can fix this problem as an extension: very useful!
    SEG_CODE = MB(0),

    // as with previous labs, we initialize 
    // our kernel heap to start at the first 
    // MB. it's 1MB, so fits in a segment. 
    SEG_HEAP = MB(1),

    // if you look in <staff-start.S>, our default
    // stack is at STACK_ADDR, so subtract 1MB to get
    // the stack start.
    SEG_STACK = STACK_ADDR - MB(1),

    // the interrupt stack that we've used all class.
    // (e.g., you can see it in the <full-except-asm.S>)
    // subtract 1MB to get its start
    SEG_INT_STACK = INT_STACK_ADDR - MB(1),

    // the base of the BCM device memory (for GPIO
    // UART, etc).  Three contiguous MB cover it.
    SEG_BCM_0 = 0x20000000,
    SEG_BCM_1 = SEG_BCM_0 + MB(1),
    SEG_BCM_2 = SEG_BCM_0 + MB(2),

    // we guarantee this (2MB) is an 
    // unmapped address
    SEG_ILLEGAL = MB(2),
};

// default kernel attributes
static inline pin_t dev_attr_default(void) {
    return pin_mk_global(dom_kern, no_user, MEM_device);
}
// default kernel attributes
static inline pin_t kern_attr_default(void) {
    return pin_mk_global(dom_kern, no_user, MEM_uncached);
}

// do default identity mapping: should use 16MB

#endif
