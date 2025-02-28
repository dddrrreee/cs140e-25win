// a very incomplete set of tests that use the arm PMU counters
// to check that different VM operations work correctly.
//
// extension: add more tests!  the counters let you figure
// out what exactly is happening for many different operations.

#include "rpi.h"
#include "procmap.h"
// in libpi/include
#include "armv6-pmu.h"
#include "cache-support.h"
#include "full-except.h"

// load a single pointer: should see one dcache miss if 
// not in cache.
static void
dcache_should_miss(const char *msg, volatile uint32_t *ptr) {
    uint32_t miss, cnt;

    // compulsory miss: we've never used <*ptr>
    //
    // uses the <armv6-pmu.h> macro:
    //  - first two arguments are the two counter variables 
    //    <miss>, <cnt>  store the PMU counts in 
    //  - next two are the types of counts (defined in the header).
    //    here we check <dcache_miss> and <inst_cnt>.
    //  - then the C code to run.
    // the macro: 
    //   - initializes the pmu to track the given count
    //   - gets the current value.
    //   - runs the statement
    //   - stores the difference in the given counters.
    pmu_stmt_measure_set(miss,cnt,
        msg,
        dcache_miss, inst_cnt,
        { 
            *ptr; 
        });

    if(miss == 0)
        panic("should have at least 1 miss, have 0\n");
    else
        output("success: have %d misses (more than 0 required)\n", miss);
}

// do 5 loads: should see zero dcache misses if in cache.
static void 
dcache_should_not_miss(const char *msg, volatile uint32_t *ptr) {
    uint32_t miss, access;

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

    // there could be other accesses so can't check ==.
    if(access <  5)
        panic("expected 5 accesses have %d\n", access);
}

// call fp (is a JIT'ed routine with 128 nops + 1 bx lr).
//
// if not in icache should be 32 prefetch buffer fetches
// (128/16=32).
static void 
icache_should_miss(const char *msg, uint32_t (*fp)(uint32_t)) {
    uint32_t miss,cnt;

    // checking icache is hard we can get conflicts or pull other
    // stuff in.
    //
    // weird: it appears that only the initial prefetch buffer fetch 
    // counts as a icache miss?
    pmu_stmt_measure_set(miss,cnt,
        msg,
        icache_miss, inst_cnt,
        { 
            fp(1);
        });

    // we should have a ton of misses on fp: 
    //  128 instructions / 4 inst prefetch buff = 32 misses.
    if(miss < 32)
        panic("should have at least 32 prefetch buffer misses\n");
    else
        output("success: have more than 32 icache misses: %d\n", miss);
}

// helper to do a raw dache invalidation: if there are dirty
// blocks they will be lost (you can see b/c subsequent reads
// will return old copies)
static inline void dcache_inv(void) {
    uint32_t r=0;
    asm volatile ("mcr p15, 0, %0, c7, c6, 0" :: "r"(r));
    prefetch_flush();
}


// free address, 16mb aligned.
enum { cache_addr = MB(16) };

// based on <1-test-one-addr.c>
static void map_everything(void) {

    //****************************************************
    // 1. map kernel memory 
    //  - same as <1-test-basic.c>

    // device memory: kernel domain, no user access, 
    // memory is strongly ordered, not shared.
    pin_t dev  = pin_mk_global(dom_kern, no_user, MEM_device);
    // kernel memory: same, but is only uncached.
    pin_t kern = pin_mk_global(dom_kern, no_user, MEM_uncached);

    pin_mmu_init(~0);
    assert(!mmu_is_enabled());

    unsigned idx = 0;
    pin_mmu_sec(idx++, SEG_CODE, SEG_CODE, kern);
    pin_mmu_sec(idx++, SEG_HEAP, SEG_HEAP, kern);
    pin_mmu_sec(idx++, SEG_STACK, SEG_STACK, kern);
    pin_mmu_sec(idx++, SEG_INT_STACK, SEG_INT_STACK, kern);
    pin_mmu_sec(idx++, SEG_BCM_0, SEG_BCM_0, dev);
    pin_mmu_sec(idx++, SEG_BCM_1, SEG_BCM_1, dev);
    pin_mmu_sec(idx++, SEG_BCM_2, SEG_BCM_2, dev);

    //****************************************************
    // 2. create a single user entry:


    // for today: user entry attributes are:
    //  - non-global
    //  - user dom
    //  - user r/w permissions. 
    // for this test:
    //  - also uncached (like kernel)
    //  - ASID = 1
    enum { ASID = 1 };

    // map a single cached page with writeback allocation.
    enum { wb_alloc     =  TEX_C_B(    0b001,  1, 1) };
    pin_t attr = pin_mk_global(dom_kern, perm_rw_priv, wb_alloc);
    pin_mmu_sec(idx++, cache_addr, cache_addr, attr);
    assert(idx<=8);

    pin_set_context(ASID);
    assert(!mmu_is_enabled());
}

void notmain(void) { 
    // our standard init.
    kmalloc_init_set_start((void*)SEG_HEAP, MB(1));
    full_except_install(0);

    // 
    // sleazy hack: create a routine at runtime that has
    // 256 nops and one bx lr on the cached page.
    //
    // we do this before vm/dcache/icache on.
    volatile uint32_t *code_ptr = (void*)(cache_addr+128);
    unsigned i;
    for(i = 0; i < 256; i++)
        code_ptr[i] = 0xe320f000; // nop
    code_ptr[i] = 0xe12fff1e;   // bx lr
    dmb();
    uint32_t (*fp)(uint32_t x) = (void *)code_ptr;
    assert(fp(5) == 5);
    assert(fp(2) == 2);

    map_everything();
    assert(!mmu_is_enabled());

    mmu_enable();
    assert(mmu_is_enabled());

    volatile uint32_t *ptr = (void*)cache_addr;

    // turn all caches on.
    caches_all_on();
    assert(caches_all_on_p());

    uint32_t miss,access;

    trace("******************************************************\n");
    trace("1. test that the page table enabled dcaching correctly.\n");
    // compulsory: tests the counters.
    dcache_should_miss("first access: should see one miss", ptr);

    // now <ptr> should be in the cache.
    dcache_should_not_miss("should be cached", ptr);

    trace("******************************************************\n");
    trace("2. test that the page table enabled icaching correctly.\n");

    // get the baseline misses i think it's zero b/c of prefetch buff.
    uint32_t base_miss = 0, cnt;
    pmu_stmt_measure_set(base_miss, cnt,
        "testing miss on nop",
        icache_miss, inst_cnt,
        {
            asm volatile ("nop");
        });

    icache_should_miss("first access: should see many icache misses", fp);
    assert(caches_all_on_p());

    trace("******************************************************\n");
    trace("3. test that sync pte mods flushes the dcache.\n");
    // sync pte mods better flush the dcache.
    mmu_sync_pte_mods();
    dcache_should_miss("after pte mod: dcache should be flushed", ptr);
    icache_should_miss("after pte mod: icache should be flushed", fp);

    trace("******************************************************\n");
    trace("4. test that mmu_on/off flushes the dcache.\n");
    assert(caches_all_on_p());
    mmu_disable();
    mmu_enable();
    assert(caches_all_on_p());

    // mmu_disable should have cleaned/invalidated dcache and icache.
    dcache_should_miss("after mmu on/off: dcache should be flushed", ptr);
    icache_should_miss("after mmu on/off: icache should be flushed", fp);

    trace("******************************************************\n");
    trace("5. about to test that disable dcache invalidates correctly\n");

    // check if we lose writes to cached memory.
    for(unsigned i =0; i < 512; i++)  {
        *ptr = i;
        mmu_disable();
        mmu_enable();
        assert(caches_all_on_p());

        if(*ptr != i)
            panic("lost a write!  have %d, expected=%d!\n", *ptr,i);
    }

    trace("******************************************************\n");
    trace("6. about to test sync_pte_mods invalides dcache correctly\n");

    // check if we lose writes to cached memory.
    for(unsigned i =0; i < 512; i++)  {
        *ptr = i;
        mmu_sync_pte_mods();
        assert(caches_all_on_p());

        if(*ptr != i)
            panic("lost a write!  have %d, expected=%d!\n", *ptr,i);
    }

    trace("SUCCESS!\n");
}
