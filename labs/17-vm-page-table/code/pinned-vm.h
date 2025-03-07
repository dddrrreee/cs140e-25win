#ifndef __PINNED_VM_H__
#define __PINNED_VM_H__
// simple interface for pinned virtual memory: most of it is 
// enum and data structures.  you'll build three routines:

#include "mmu.h"
#include "libc/bit-support.h"
#include "mem-attr.h"


// you can flip these back and forth if you want debug output.
#if 0
    // change <output> to <debug> if you want file/line/func
#   define pin_debug(args...) output("PIN_VM:" args)
#else
#   define pin_debug(args...) do { } while(0)
#endif

// attributes: these get inserted into the TLB.
typedef struct {
    // for today we only handle 1MB sections.
    uint32_t G,         // is this a global entry?
             asid,      // don't think this should actually be in this.
             dom,       // domain id
             pagesize;  // can be 1MB or 16MB

    // permissions for needed to access page.
    //
    // see mem_perm_t above: is the bit merge of 
    // APX and AP so can or into AP position.
    mem_perm_t  AP_perm;

    // caching policy for this memory.
    // 
    // see mem_cache_t enum above.
    // see table on 6-16 b4-8/9
    // for today everything is uncacheable.
    mem_attr_t mem_attr;
} pin_t;


// pattern for overriding values.
static inline pin_t
pin_mem_attr_set(pin_t e, mem_attr_t mem) {
    e.mem_attr = mem;
    return e;
}

// pinned encodings for different page sizes.
enum {
    PAGE_4K     = 0b01,
    PAGE_64K    = 0b10,
    PAGE_1MB    = 0b11,
    PAGE_16MB   = 0b00
};

// set <p> to be a 16MB page.
static inline pin_t pin_16mb(pin_t p) {
    p.pagesize = PAGE_16MB;
    return p;
}
// set <p> to be a 64k page
static inline pin_t pin_64k(pin_t p) {
    p.pagesize = PAGE_64K;
    return p;
}

// possibly we should just multiply these.
enum {
    _4k     = 4*1024, 
    _64k    = 64*1024,
    _1mb    = 1024*1024,
    _16mb   = 16*_1mb
};

// return the number of bytes for <attr>
static inline unsigned
pin_nbytes(pin_t attr) {
    switch(attr.pagesize) {
    case 0b01: return _4k;
    case 0b10: return _64k;
    case 0b11: return _1mb;
    case 0b00: return _16mb;
    default: panic("invalid pagesize\n");
    }
}

// check page alignment. as is typical, arm1176
// requires 1mb pages to be aligned to 1mb; 16mb 
// aligned to 16mb, etc.
static inline unsigned 
pin_aligned(uint32_t va, pin_t attr) {
    unsigned n = pin_nbytes(attr);
    switch(n) {
    case _4k:   return va % _4k == 0;
    case _64k:  return va % _64k == 0;
    case _1mb:  return va % _1mb == 0;
    case _16mb: return va % _16mb == 0;
    default: panic("invalid size=%u\n", n);
    }
}

//****************************************************************
// constructors for common pin attributes:
//  - device, kernel, user private.


// create the generic mapping attribute structure.
static inline pin_t
pin_mk(uint32_t G,
            uint32_t dom,
            uint32_t asid,
            mem_perm_t perm, 
            mem_attr_t mem_attr) 
{
    demand(mem_perm_islegal(perm), "invalid permission: %b\n", perm);
    demand(dom <= 16, illegal domain id);

    if(G)
        demand(!asid, "should not have a non-zero asid: %d", asid);
    else {
        demand(asid, non-global: should have non-zero asid);
        demand(asid > 0 && asid < 64, illegal asid);
    }

    return (pin_t) {
            .dom = dom,
            .asid = asid,
            .G = G,
            // default: 1MB section.
            .pagesize = 0b11,
            .mem_attr = mem_attr,
            .AP_perm = perm 
    };
}

// make a global entry: 
//  - global bit se [asid=0]
//  - non-cacheable
//  - default: 1mb section.
static inline pin_t
pin_mk_global(uint32_t dom, mem_perm_t perm, mem_attr_t attr) {
    // G=1, asid=0.
    return pin_mk(1, dom, 0, perm, attr);
}

// private user mapping: 
//  - global=0
//  - asid != 0
//  - domain should not be kernel domain (we don't check)
static inline pin_t
pin_mk_user(uint32_t dom, uint32_t asid, mem_perm_t perm, mem_attr_t attr) {
    return pin_mk(0, dom, asid, perm, attr);
}

// make a dev entry: 
//  - global bit set [asid=0]
//  - non-cacheable
//  - kernel R/W, user no-access
//  - default: 1mb section.
static inline pin_t pin_mk_device(uint32_t dom) {
    return pin_mk(1, dom, 0, perm_rw_priv, MEM_device);
}

/******************************************************************
 * routines for pinnin: you'll implement these
 */

// call to initialize the MMU hardware.
void pin_mmu_init(uint32_t domain_reg);
void staff_pin_mmu_init(uint32_t domain_reg);

// simple wrappers
static inline void pin_mmu_enable(void) {
    assert(!mmu_is_enabled());
    staff_mmu_enable();
    assert(mmu_is_enabled());
}
static inline void pin_mmu_disable(void) {
    assert(mmu_is_enabled());
    staff_mmu_disable();
    assert(!mmu_is_enabled());
}


// enable MMU -- must have set the context
// first.
void pin_mmu_enable(void);
void staff_pin_mmu_enable(void);

// disable MMU
void pin_mmu_disable(void);
void staff_pin_mmu_disable(void);

// mmu can be off or on: setup address space context.
// must do before enabling MMU or when switching
// between address spaces.
void staff_pin_set_context(uint32_t asid);
void pin_set_context(uint32_t asid);

// our main routine: 
//  insert map <va> ==> <pa> with attributes <attr> 
//  at position <idx> in the TLB.
//
// errors:
//  - idx >= 8.
//  - va already mapped.
void pin_mmu_sec(unsigned idx,
                uint32_t va,
                uint32_t pa,
                pin_t attr);

void staff_pin_mmu_sec(unsigned idx,
                uint32_t va,
                uint32_t pa,
                pin_t attr);


// do a manual translation in tlb and see if exists (1)
// returns the result in <result>
//
// low 1..6 bits of result should have the reason.
int tlb_contains_va(uint32_t *result, uint32_t va);
int staff_tlb_contains_va(uint32_t *result, uint32_t va);

// wrapper that check that <va> is pinned in the tlb
int pin_exists(uint32_t va, int verbose_p);
int staff_pin_exists(uint32_t va, int verbose_p);

// print <msg> then all valid pinned entries.
// note: if you print everything you see a lot of
// garbage in the non-initialized entires --- i'm 
// not sure if we are guaranteed that these have their
// valid bit set to 0?   
void lockdown_print_entries(const char *msg);

// set pin index <idx> to all 0s.
void pin_clear(unsigned idx);
void staff_pin_clear(unsigned idx);

#endif
