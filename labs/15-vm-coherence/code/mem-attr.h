#ifndef __MEM_ATTR_H__
#define __MEM_ATTR_H__
// common memory attributes.

#include "mmu.h"
#include "libc/bit-support.h"

// this enum flag is a three bit value
//      AXP:1 << 2 | AP:2 
// so that you can just bitwise or it into the 
// position for AP (which is adjacent to AXP).
//
// if _priv access = kernel only.
// if _user access, implies kernel access (but
// not sure if this should be default).
//
// see: 3-151 for table or B4-9 for more 
// discussion.
typedef enum {
    perm_rw_user = 0b011, // read-write user 
    perm_ro_user = 0b010, // read-only user
    perm_na_user = 0b001, // no access user

    // kernel only, user no access
    perm_ro_priv = 0b101,
    // perm_rw_priv = perm_na_user,
    perm_rw_priv = perm_na_user,
    perm_na_priv = 0b000,
} mem_perm_t;

static inline int mem_perm_islegal(mem_perm_t p) {
    switch(p) {
    case perm_rw_user:
    case perm_ro_user:
    // case perm_na_user:
    case perm_ro_priv:
    case perm_rw_priv:
    case perm_na_priv:
        return 1;
    default:
        // for today just die.
        panic("illegal permission: %b\n", p);
    }
}

// domain permisson enums: see b4-10
enum {
    DOM_no_access   = 0b00, // any access = fault.
    // client accesses check against permission bits in tlb
    DOM_client      = 0b01,
    // don't use.
    // DOM_reserved    = 0b10,
    // TLB access bits are ignored.
    DOM_manager     = 0b11,
};

// from Table 6-2 on 6-15:
//
// caching is controlled by the TEX, C, and B bits.
// these are laid out contiguously:
//      TEX:3 | C:1 << 1 | B:1
// we construct an enum with a value that maps to
// the description so that we don't have to pass
// a ton of parameters.
//
#define TEX_C_B(tex,c,b)  ((tex) << 2 | (c) << 1 | (b))
typedef enum { 
    //                              TEX   C  B 
    // strongly ordered
    // not shared.
    MEM_device     =  TEX_C_B(    0b000,  0, 0),  
    // normal, non-cached
    MEM_uncached   =  TEX_C_B(    0b001,  0, 0),  

    // write back no alloc
    MEM_wb_noalloc =  TEX_C_B(    0b000,  1, 1),  
    // write through no alloc
    MEM_wt_noalloc =  TEX_C_B(    0b000,  1, 0),  

    // NOTE: missing a lot!
} mem_attr_t;
#endif
