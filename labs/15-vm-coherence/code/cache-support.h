#ifndef __CACHE_SUPPORT_H__
#define __CACHE_SUPPORT_H__
// turn on the different caches by changing 
// co-processor 15.  
//
// note: unclear if the r/pi lets the CPU use the
// L2 cache directly.  if it does there is some
// additional configuration we need to do.
#include "armv6-cp15.h"

// libpi/staff-src/cache-support.S
// flushes tlb, icache dcache etc.
void cache_flush_all(void);

// you have to define this.
int mmu_is_enabled(void);

// enable btc
static inline int btc_is_on(void) {
    let c = cp15_ctrl_reg1_rd();
    return c.Z_branch_pred == 1;
}

// branch target cache.
static inline void btc_on(void) {
    assert(mmu_is_enabled());
    let c = cp15_ctrl_reg1_rd();
    c.Z_branch_pred = 1;
    cp15_ctrl_reg1_wr(c);
    assert(btc_is_on());
}

static inline int dcache_l1_is_on(void) {
    let c = cp15_ctrl_reg1_rd();
    return c.C_unified_enable == 1;
}

// enable just the L1 cache: doesn't work
// without mmu on.
static inline void dcache_l1_on(void) {
    assert(mmu_is_enabled());
    let c = cp15_ctrl_reg1_rd();
    // assert(c.C_unified_enable == 0);
    c.C_unified_enable = 1;
    cp15_ctrl_reg1_wr(c);
    assert(dcache_l1_is_on());
}


// enable dcache writeback: doesn't work 
// without mmu on.
static inline int dcache_wb_is_on(void) {
    let c = cp15_ctrl_reg1_rd();
    return c.W_write_buf == 1;
}

static inline void dcache_wb_on(void) {
    assert(mmu_is_enabled());
    let c = cp15_ctrl_reg1_rd();
    c.W_write_buf = 1;
    cp15_ctrl_reg1_wr(c);

    assert(dcache_wb_is_on());
}


static inline int dcache_l2_is_on(void) {
    let c = cp15_ctrl_reg1_rd();
    return c.L2_enabled == 1;
}

// enable l2 cache.
static inline void dcache_l2_on(void) {
    panic("need to do extra stuff to enable\n");
    assert(mmu_is_enabled());
    let c = cp15_ctrl_reg1_rd();
    c.L2_enabled = 1;
    cp15_ctrl_reg1_wr(c);
    assert(dcache_l2_is_on());
}

// disable all caches: note, you better make
// sure you flush and invalidate everything
// first.
static inline void caches_all_off(void) {
    // i think you need to flush it first.  interesting
    // bug.
    cache_flush_all();

    let c = cp15_ctrl_reg1_rd();
    c.C_unified_enable = 0;
    c.W_write_buf = 0;
    c.L2_enabled = 0;
    c.Z_branch_pred = 0;
    c.I_icache_enable = 0;
    cp15_ctrl_reg1_wr(c);
}

static inline int icache_is_on(void) {
    let c = cp15_ctrl_reg1_rd();
    return c.I_icache_enable == 1;
}
static inline void icache_on(void) {
    let c = cp15_ctrl_reg1_rd();
    c.I_icache_enable = 1;
    cp15_ctrl_reg1_wr(c);

    assert(icache_is_on());
}

static inline int caches_all_on_p(void) {
    return 
        btc_is_on()
        && dcache_wb_is_on()
        && dcache_l1_is_on()
        && icache_is_on();
}

// enable all caches.
static inline void caches_all_on(void) {
    assert(mmu_is_enabled());

    icache_on();
    btc_on();
    dcache_l1_on();
    dcache_wb_on();
    // dcache_l2_on();

#if 0
    // faster way but we do the slow way
    // for internal consistency checking
    let c = cp15_ctrl_reg1_rd();
    c.C_unified_enable = 1;
    c.W_write_buf = 1;
    c.L2_enabled = 1;
    c.Z_branch_pred = 1;
    c.I_icache_enable = 1;
    cp15_ctrl_reg1_wr(c);
#endif
    
    assert(caches_all_on_p());
}

#endif
