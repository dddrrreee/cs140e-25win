#ifndef __PINNED_VM_H__
#define __PINNED_VM_H__
// simple interface for pinned virtual memory: most of it is 
// enum and data structures.  you'll build three routines:

#include "mmu.h"

#include "libc/bit-support.h"


// you can flip these back and forth if you want debug output.
#if 0
    // change <output> to <debug> if you want file/line/func
#   define pin_debug(args...) output("PIN_VM:" args)
#else
#   define pin_debug(args...) do { } while(0)
#endif

// kinda iffy to have this here...
enum { MB = 1024 * 1024 };

// kernel domain = 1.  we leave 0 free so can sanity check.
enum { kern_dom = 1 };

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
    DOM_reserved    = 0b10,
    // TLB access bits are ignored.
    DOM_manager     = 0b11,
};

// caching is controlled by the TEX, C, and B bits.
// these are laid out contiguously:
//      TEX:3 | C:1 << 1 | B:1
// we construct an enum with a value that maps to
// the description so that we don't have to pass
// a ton of parameters.
//
// from table 6-2 on 6-15:
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

// default: 
//  - 1mb section
static inline pin_t
pin_mk(uint32_t G,
            uint32_t dom,
            uint32_t asid,
            mem_perm_t perm, 
            mem_attr_t mem_attr) {
    demand(mem_perm_islegal(perm), "invalid permission: %b\n", perm);
    demand(dom <= 16, illegal domain id);
    if(G)
        demand(!asid, "should not have a non-zero asid: %d", asid);
    else {
        demand(asid, should have non-zero asid);
        demand(asid > 0 && asid < 64, invalid asid);
    }


    return (pin_t) {
            .dom = dom,
            .asid = asid,
            .G = G,
            // default: 1MB section.
            .pagesize = 0b11,
            // default: uncacheable
            // .mem_attr = MEM_uncached, // mem_attr,
            .mem_attr = mem_attr,
            .AP_perm = perm };
}

// pattern for overriding values.
static inline pin_t
pin_mem_attr_set(pin_t e, mem_attr_t mem) {
    e.mem_attr = mem;
    return e;
}

// encoding for different page sizes.
enum {
    PAGE_4K     = 0b01,
    PAGE_64K    = 0b10,
    PAGE_1MB    = 0b11,
    PAGE_16MB   = 0b00
};

// override default <p>
static inline pin_t pin_16mb(pin_t p) {
    p.pagesize = PAGE_16MB;
    return p;
}
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

// make a dev entry: 
//    - global bit set.
//    - non-cacheable
//    - kernel R/W, user no-access
static inline pin_t pin_mk_device(uint32_t dom) {
    return pin_mk(1, dom, 0, perm_rw_priv, MEM_device);
}


// make a global entry: 
//  - global bit set
//  - asid = 0
//  - default: 1mb section.
static inline pin_t
pin_mk_global(uint32_t dom, mem_perm_t perm, mem_attr_t attr) {
    // G=1, asid=0.
    return pin_mk(1, dom, 0, perm, attr);
}

// must have:
//  - global=0
//  - asid != 0
//  - domain should not be kernel domain (we don't check)
static inline pin_t
pin_mk_user(uint32_t dom, uint32_t asid, mem_perm_t perm, mem_attr_t attr) {
    return pin_mk(0, dom, asid, perm, attr);
}

/******************************************************************
 * routines for pinnin: you'll implement these three.
 */

// i think pinned should just provide pin_mmu_sec and pin_t and a 
// handful of helpers.  the mmu turn on and the procmap stuff is
// in the support code.

// our main routine: map <va>-><pa> with 
// attributes <attr> at position <idx> in the
// TLB.
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
int tlb_contains_va(uint32_t *result, uint32_t va);
int staff_tlb_contains_va(uint32_t *result, uint32_t va);


// turn mmu on for the first time: empty_pt is just a 4k vector
// of 0s so we get a fault if it's ever examined.
void staff_mmu_on_first_time(uint32_t asid, void *empty_pt);
void mmu_on_first_time(uint32_t asid, void *empty_pt);


#if 0
// turn pinned MMU on.
void pin_mmu_on(procmap_t *p);
void staff_pin_mmu_on(procmap_t *p);
#endif

// this is dumb: make it return a bool.
// check that <va> is pinned in the tlb.
void pin_check_exists(uint32_t va);
void staff_pin_check_exists(uint32_t va);

// print <msg> then all valid pinned entries.
// note: if you print everything you see a lot of
// garbage in the non-initialized entires --- i'm 
// not sure if we are guaranteed that these have their
// valid bit set to 0?   
void lockdown_print_entries(const char *msg);

#if 0
// identity map (addr->addr) the device memory with 
// right cache and permission:
//    -global bit set.
//    - non-cacheable
//    - kernel R/W, user no-access
static inline void
pin_ident_map_device(uint32_t idx, uint32_t addr, uint32_t dom) {
    // we use ASID = 0 b/c is global.
    staff_mmu_pin_sec(idx, addr, addr, pin_mk_device(dom));
}
#endif

// clear pin index <idx> to all 0s.
void pin_clear(unsigned idx);

void staff_pin_clear(unsigned idx);

void staff_pin_init(void) ;
void pin_init(void) ;

// these don't do much today.
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

// mmu can be off or on.
void pin_mmu_switch(uint32_t pid, uint32_t asid);
void staff_pin_mmu_switch(uint32_t pid, uint32_t asid);

void staff_pin_mmu_init(uint32_t domain_reg);
void pin_mmu_init(uint32_t domain_reg);

#endif
