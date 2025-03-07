// first test do low level setup of everything.
#include "rpi.h"
#include "pt-vm.h"
#include "armv6-pmu.h"

enum { OneMB = 1024*1024, cache_addr = OneMB * 10 };

static void enable_l1(void) {
    assert(mmu_is_enabled());
    let c = cp15_ctrl_reg1_rd();
    // assert(c.C_unified_enable == 0);
    c.C_unified_enable = 1;
    cp15_ctrl_reg1_wr(c);
}

static void enable_wb(void) {
    assert(mmu_is_enabled());
    let c = cp15_ctrl_reg1_rd();
    c.W_write_buf = 1;
    cp15_ctrl_reg1_wr(c);
}

static void enable_l2(void) {
    assert(mmu_is_enabled());
    let c = cp15_ctrl_reg1_rd();
    c.L2_enabled = 1;
    cp15_ctrl_reg1_wr(c);
}

void flush_caches(void);

static void caches_disable_all(void) {
    let c = cp15_ctrl_reg1_rd();
    c.C_unified_enable = 0;
    c.W_write_buf = 0;
    c.L2_enabled = 0;
    c.Z_branch_pred = 0;
    c.I_icache_enable = 0;
    cp15_ctrl_reg1_wr(c);
    flush_caches();
}

static void caches_enable_all(void) {
    assert(mmu_is_enabled());

    let c = cp15_ctrl_reg1_rd();
    c.C_unified_enable = 1;
    c.W_write_buf = 1;
    c.L2_enabled = 1;
    c.Z_branch_pred = 1;
    c.I_icache_enable = 1;
    cp15_ctrl_reg1_wr(c);
}

#if 0
    unsigned r;
    asm volatile ("MRC p15, 0, %0, c1, c0, 0" : "=r" (r));
    r |= (1 << 12); // l1 instruction cache
    r |= (1 << 11); // branch prediction
    asm volatile ("MCR p15, 0, %0, c1, c0, 0" :: "r" (r));
#endif

/******************************************************************
 * icache
 */

void measure_dcache(const char *msg) {
    volatile uint32_t *ptr = (void*)cache_addr;

    unsigned cycle1 = pmu_event1_get();

    unsigned miss0  = pmu_event0_get();
    for(int i = 0; i < 10; i++) {
        *ptr; *ptr; *ptr; *ptr; *ptr;
        *ptr; *ptr; *ptr; *ptr; *ptr;
        *ptr; *ptr; *ptr; *ptr; *ptr;
        *ptr; *ptr; *ptr; *ptr; *ptr;
    }
    unsigned miss0_end  = pmu_event0_get();

    unsigned cycle1_end = pmu_event1_get();
    output("\t%s: total %s=%d, total %s=%d\n",
        msg,
        "misses", miss0_end - miss0,
        "cycles", cycle1_end - cycle1);
}

void run_dcache_test(void) {
    // caches_disable_all();

#if 0
    // enable icache and bt.
    caches_enable();
#endif

    pmu_enable0(PMU_DCACHE_MISS);
    pmu_enable1(PMU_CYCLE_CNT);
    measure_dcache("cache disabled run=1");
    measure_dcache("cache disabled run=2");
    measure_dcache("cache disabled run=3");
    output("\t----------------------------------------\n");

    caches_enable_all();
#if 0
    enable_l1();
    enable_l2();
    enable_wb();
#endif
    measure_dcache("cache enabled  run=1");
    measure_dcache("cache enabled  run=2");
    measure_dcache("cache enabled  run=3");
    output("\t----------------------------------------\n");
    flush_caches();
    measure_dcache("after flush    run=1");

    flush_caches();
    measure_dcache("after flush    run=2");

    flush_caches();
    measure_dcache("after flush    run=3");
    output("\t----------------------------------------\n");

    measure_dcache("after no flush run=1");
    measure_dcache("after no flush run=2");
    measure_dcache("after no flush run=3");
    output("\t----------------------------------------\n");
}
/******************************************************************
 * icache
 */

void measure_icache(const char *msg) {
    unsigned cycle1 = pmu_event1_get();

    unsigned miss0  = pmu_event0_get();
    for(int i = 0; i < 10; i++) {
        asm volatile ("nop");  // 1
        asm volatile ("nop");  // 2
        asm volatile ("nop");  // 3
        asm volatile ("nop");  // 4
        asm volatile ("nop");  // 5

        asm volatile ("nop");  // 1
        asm volatile ("nop");  // 2
        asm volatile ("nop");  // 3
        asm volatile ("nop");  // 4
        asm volatile ("nop");  // 5

        asm volatile ("nop");  // 1
        asm volatile ("nop");  // 2
        asm volatile ("nop");  // 3
        asm volatile ("nop");  // 4
        asm volatile ("nop");  // 5

        asm volatile ("nop");  // 1
        asm volatile ("nop");  // 2
        asm volatile ("nop");  // 3
        asm volatile ("nop");  // 4
        asm volatile ("nop");  // 5
    }
    unsigned miss0_end  = pmu_event0_get();

    unsigned cycle1_end = pmu_event1_get();
    output("\t%s: total %s=%d, total %s=%d\n",
        msg,
        "misses", miss0_end - miss0,
        "cycles", cycle1_end - cycle1);
}

void run_icache_test(void) {
    caches_disable_all();

    pmu_enable0(PMU_ICACHE_MISS);
    pmu_enable1(PMU_CYCLE_CNT);
    measure_icache("cache disabled run=1");
    measure_icache("cache disabled run=2");
    measure_icache("cache disabled run=3");
    output("\t----------------------------------------\n");

    caches_enable();
    measure_icache("cache enabled  run=1");
    measure_icache("cache enabled  run=2");
    measure_icache("cache enabled  run=3");
    output("\t----------------------------------------\n");
    flush_caches();
    measure_icache("after flush    run=1");

    flush_caches();
    measure_icache("after flush    run=2");

    flush_caches();
    measure_icache("after flush    run=3");
    output("\t----------------------------------------\n");

    measure_icache("after no flush run=1");
    measure_icache("after no flush run=2");
    measure_icache("after no flush run=3");
    output("\t----------------------------------------\n");
}

void notmain(void) { 
    // map the heap: for lab cksums must be at 0x100000.
    kmalloc_init_set_start((void*)OneMB, OneMB);
    assert(!mmu_is_enabled());

    enum { dom_kern = 1,
           // domain for user = 2
           dom_user = 2 };          

    procmap_t p = procmap_default_mk(dom_kern);
    vm_pt_t *pt = vm_map_kernel(&p,0);
    assert(!mmu_is_enabled());


    // map a single cached page with writeback allocation.
    // write back, write alloc
    enum { wb_alloc     =  TEX_C_B(    0b001,  1, 1) };

    pin_t attr = pin_mk_global(dom_kern, perm_rw_priv, wb_alloc);
    vm_map_sec(pt, cache_addr, cache_addr, attr);

    pmu_itlb_miss_on(0);
    pmu_dtlb_miss_on(1);

    uint32_t itlb_start = pmu_itlb_miss(0);
    uint32_t dtlb_start = pmu_dtlb_miss(1);
    uint32_t cycles_start = pmu_cycle_get();

    vm_mmu_enable();
    assert(mmu_is_enabled());

    uint32_t n_cycles = pmu_cycle_get() - cycles_start;
    uint32_t n_itlb = pmu_itlb_miss(0) - itlb_start;
    uint32_t n_dtlb = pmu_dtlb_miss(1) - dtlb_start;
    output("cycles=%d, dtlb misses = %d, itlb misses = %d\n", 
        n_cycles, n_dtlb, n_itlb);

#if 0

// we could make it so it records the current
// and then restores it.  not sure if this 
// makes any sense.
#define pmu_stmt_measure(msg, type0, type1, stmts) do { \
    pmu_ ## type0 ## _on(0);                            \
    pmu_ ## type1 ## _on(1);                            \
    uint32_t ty0 = pmu_ ## type0(0);                    \
    uint32_t ty1 = pmu_ ## type1(1);                    \
    uint32_t cyc = pmu_cycle_get();                     \
                                                        \
    /* partially stop gcc from moving stuff */          \
    gcc_mb();                                           \
    stmts;                                              \
    gcc_mb();                                           \
                                                        \
    uint32_t n_ty0 = pmu_ ## type0(0) - ty0;            \
    uint32_t n_ty1 = pmu_ ## type1(1) - ty1;            \
    uint32_t n_cyc = pmu_cycle_get() -  cyc;            \
    const char *s0 = pmu_ ## type0 ## _str();           \
    const char *s1 = pmu_ ## type1 ## _str();           \
                                                        \
    output("%s:%d: %s:\n\tcycles=%d\n\t%s=%d\n\t%s=%d\n", \
        __FILE__, __LINE__, msg, n_cyc, s0, n_ty0, s1, n_ty1);              \
} while(0)

#endif
    // interesting: the tlb doesn't seem to get hit 
    // for code translation sometimes?  after runs once.
    // wait i think this is wrong.

    volatile uint32_t *ptr = (void*)cache_addr;
    int x;
    staff_sync_tlb();
    pmu_stmt_measure("measuring tlb misses after invalidation", 
        itlb_miss, dtlb_miss, 
        { 
            staff_sync_tlb();
            *ptr;
            staff_sync_tlb();
            *ptr;
            staff_sync_tlb();
            *ptr;
            staff_sync_tlb();
            *ptr;
        } 
    );
    clean_reboot();

#if 0
    flush_caches();
    cp15_itlb_inv();
    cp15_dtlb_inv();
    cp15_tlbs_inv();
#endif


    // run_icache_test();
//    run_icache_test();
    run_dcache_test();

    trace("SUCCESS!\n");
}
