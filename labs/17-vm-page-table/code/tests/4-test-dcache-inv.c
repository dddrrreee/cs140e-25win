// first test do low level setup of everything.
#include "rpi.h"
#include "pt-vm.h"
#include "armv6-pmu.h"
#include "cache-support.h"

enum { OneMB = 1024*1024, cache_addr = OneMB * 10 };

void notmain(void) { 
    // sleazy hack: create a bx lr nop routine on the page.
    volatile uint32_t *code_ptr = (void*)(cache_addr+128);
    unsigned i;
    for(i = 0; i < 256; i++)
        code_ptr[i] = 0xe320f000; // nop
    code_ptr[i] = 0xe12fff1e;   // bx lr
    dmb();

    // should probably make a bunch of adds.
    uint32_t (*fp)(uint32_t x) = (void *)code_ptr;
    assert(fp(5) == 5);
    assert(fp(2) == 2);

    // map the heap: for lab cksums must be at 0x100000.
    kmalloc_init_set_start((void*)OneMB, OneMB);
    enum { dom_kern = 1 };

    procmap_t p = procmap_default_mk(dom_kern);
    vm_pt_t *pt = vm_map_kernel(&p,0);
    assert(!mmu_is_enabled());

    // should also try it after.
    pin_t attr = pin_mk_global(dom_kern, perm_rw_priv, MEM_wb_alloc);


    vm_map_sec(pt, cache_addr, cache_addr, attr);
    vm_mmu_enable();
    assert(mmu_is_enabled());

    volatile uint32_t *ptr = (void*)cache_addr;


    // turn all caches on.
    caches_all_on();
    assert(caches_all_on_p());


    uint32_t miss,access;

    /*****************************************************************
     * 1
     */

    output("******************************************************\n");
    output("1. test that the page table enabled dcaching correctly.\n");

    // compulsory miss: we've never used <*ptr>
    pmu_stmt_measure_set(miss,access,
        "first access: should see one miss", 
        dcache_miss, dcache_access,
        { 
            *ptr; 
        });

    if(miss == 0)
        panic("should have at least 1 miss, have 0\n");

    // should not miss.
    pmu_stmt_measure_set(miss, access, 
        "should be cached: should see no misses", 
        dcache_miss, dcache_access,
        { 
            *ptr;  /* 1 */
            *ptr;  /* 2 */
            *ptr;  /* 3 */
            *ptr;  /* 4 */
            *ptr;  /* 5 */
        });
    if(miss > 0)
        panic("should have had zero misses: have %d\n", miss);
    if(access <  5)
        panic("expected 5 accesses have %d\n", access);

    output("******************************************************\n");
    output(" test that the page table enabled icaching correctly.\n");

    // get the base line misses [how is it 0?].
    uint32_t base_miss = 0, cnt;
    pmu_stmt_measure_set(base_miss, cnt,
        "testing miss on nop",
        icache_miss, inst_cnt,
        {
            asm volatile ("nop");
        });

    // checking icache is hard we can get conflicts or pull other
    // stuff in.
    //
    // weird: it appears that only the initial prefetch buffer fetch 
    // counts as a icache miss?
    pmu_stmt_measure_set(miss,cnt,
        "first access: should see one miss", 
        icache_miss, inst_cnt,
        { 
            fp(1);
        });

    // we should have a ton of misses on fp: 
    //  128 instructions / 4 = 32
    output("base-miss=%d, miss=%d\n", base_miss,miss);
    if((miss-base_miss) < 32)
        panic("should have at least 32 prefetch buffer misses\n");

    assert(caches_all_on_p());

    /**********************************************************
     * 2. test that sync pte mods flushes the dcache.
     */

    output("******************************************************\n");
    output("2. test that sync pte mods flushes the dcache.\n");

    // sync pte mods better flush the dcache.
    mmu_sync_pte_mods();
    pmu_stmt_measure_set(miss, access, 
        "dcache should be flushed: expect 1 dcache miss", 
        dcache_miss, dcache_access,
        { 
            *ptr; 
        });

    // this is making strong assumptions about gcc optimization.
    if(miss != 1)
        panic("should have had 1 miss: have %d\n", miss);

    /**********************************************************
     * 3. test that mmu_on/off flushes the dcache.
     */

    output("******************************************************\n");
    output("3. test that mmu_on/off flushes the dcache.\n");
    assert(caches_all_on_p());
    vm_mmu_disable();
    vm_mmu_enable();
    assert(caches_all_on_p());

    pmu_stmt_measure_set(miss, access, 
        "dcache should be flushed: expect 1 miss", 
        dcache_miss, dcache_access,
        { 
            *ptr; 
        });


    // this is making strong assumptions about gcc optimization.
    if(miss != 1)
        panic("should have had 1 miss: have %d\n", miss);

    trace("SUCCESS!\n");
}
