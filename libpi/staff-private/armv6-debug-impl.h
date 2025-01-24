#ifndef __ARMV6_DEBUG_IMPL_H__
#define __ARMV6_DEBUG_IMPL_H__
#include "armv6-debug.h"

// all your code should go here.  implementation of the debug interface.
coproc_mk(status, p14, 0, c0, c1, 0)

// return 1 if enabled, 0 otherwise.  
//    - we wind up reading the status register a bunch:
//      could return its value instead of 1 (since is 
//      non-zero).
static inline int cp14_is_enabled(void) {
    uint32_t s = cp14_status_get();
    return !bit_isset(s, 14) && bit_isset(s, 15);
}

// enable debug coprocessor 
static inline void cp14_enable(void) {
    // if it's already enabled, just return?
    if(cp14_is_enabled())
        panic("already enabled\n");

    // for the core to take a debug exception, monitor debug mode has to be both 
    // selected and enabled --- bit 14 clear and bit 15 set.
    uint32_t s = cp14_status_get();

    static int first_time = 1;
    if(first_time) {
        // when we start up these must be true.
        assert(!bit_isset(s, 14));
        assert(!bit_isset(s, 15));
        first_time = 0;
    }

    cp14_status_set(bit_set(s, 15));
    prefetch_flush();

    assert(cp14_is_enabled());
}

// disable debug coprocessor
static inline void cp14_disable(void) {
    if(!cp14_is_enabled())
        return;

    uint32_t s = cp14_status_get();
    cp14_status_set(bit_clr(s, 15));
    prefetch_flush();

    assert(!cp14_is_enabled());
}

// 13-26
coproc_mk(bcr0, p14, 0, c0, c0, 5)
coproc_mk(bvr0, p14, 0, c0, c0, 4)

coproc_mk(dscr, p14, 0, c0, c1, 0)
coproc_mk(wfar, p14, 0, c0, c6, 0)

coproc_mk(wvr0, p14, 0, c0, c0, 6)
coproc_mk(wcr0, p14, 0, c0, c0, 7)

static inline int cp14_bcr0_is_enabled(void) {
    uint32_t b = cp14_bcr0_get();
    return bit_isset(b, 0);
}
static inline void cp14_bcr0_enable(void) {
    cp14_bcr0_set(cp14_bcr0_get() | 1);
}
static inline void cp14_bcr0_disable(void) {
    cp14_bcr0_set(bit_clr(cp14_bcr0_get(), 0));
}

// 3-68
coproc_mk_get(ifsr, p15, 0, c5, c0, 1)
// 3-68: holds address fault occured at.
coproc_mk_get(ifar, p15, 0, c6, c0, 2)

// was this a brkpt fault?
static inline int was_brkpt_fault(void) {
    uint32_t r = cp15_ifsr_get();
    // "these bits are not set on debug event.
    if(bit_is_on(r, 10) || bits_get(r, 0, 4) != 0b0010)
        return 0;


    // more precise cause
    uint32_t dscr = cp14_dscr_get();
    // XXX: turn these into enums
    if(bits_get(dscr, 2, 5) == 0b0001)
        return 1;
    return 0;
}

// see 3-65 
coproc_mk_get(dfsr, p15, 0, c5, c0, 0)

coproc_mk_get(far, p15, 0, c6, c0, 0)


// was watchpoint debug fault caused by a load?
static inline int datafault_from_ld(void) {
    return bit_isset(cp15_dfsr_get(), 11) == 0;
}
// ...  by a store?
static inline int datafault_from_st(void) {
    return !datafault_from_ld();
}


// 13-33: tabl 13-23
static inline int was_watchpt_fault(void) {
    uint32_t r = cp15_dfsr_get();
    // "these bits are not set on debug event.
    if(bit_is_on(r, 10) || bits_get(r, 0, 4) != 0b0010)
        return 0;

    // 13-11: watchpoint occured: bits [5:2] == 0b0010
    return bits_get(cp14_dscr_get(), 2, 5) == 0b0010;
}

static inline int cp14_wcr0_is_enabled(void) {
    uint32_t b = cp14_wcr0_get();
    return bit_isset(b, 0);
}
static inline void cp14_wcr0_enable(void) {
    uint32_t b = cp14_wcr0_get();
    cp14_wcr0_set(b|1);
}
static inline void cp14_wcr0_disable(void) {
    uint32_t b = cp14_wcr0_get();
    cp14_wcr0_set(bit_clr(b,0));
}


// adjust according to table.
static inline uint32_t watchpt_fault_pc(void) {
    return cp14_wfar_get() - 8;
}
static inline uint32_t watchpt_fault_addr(void) {
    return cp15_far_get();
}
    
#endif
