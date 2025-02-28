// same as <1-test-basic.c> but we add a single 
// user mapping (map same VA to PA so is an "identity
// mapping")
//
// we also use the enums in <memmap-default.h> which
// defines:
// - the default segments and <no_user>
//   (identical to <1-test-basic.c>)
// - dom_kern and dom_user:
//   enum { 
//      dom_kern = 1, // domain id for kernel
//      dom_user = 2  // domain id for user
//   };          
//
// we also switch to using the pinned calls.
#include "rpi.h"
#include "full-except.h"
#include "pinned-vm.h"
#include "mmu.h"
#include "memmap-default.h"

void notmain(void) { 
    // our standard init.
    kmalloc_init_set_start((void*)SEG_HEAP, MB(1));
    full_except_install(0);

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
    pin_t usr = pin_mk_user(dom_user, ASID, user_access, MEM_uncached);
    
    // identity map 16th MB for r/w
    enum { 
        user_va_addr = MB(16),
        user_pa_addr = MB(16),
    };
    pin_mmu_sec(idx++, user_va_addr, user_pa_addr, usr);
    assert(idx<=8);

    // initialize before turning vm on.
    PUT32(user_pa_addr, 0xdeadbeef);

    //****************************************************
    // 3. now turn on the MMU and make sure the expected
    //    entries are there.

    trace("about to enable\n");
    lockdown_print_entries("about to turn on first time");

    // setup address space context before turning on.
    pin_set_context(ASID);

    // make sure no helper enabled by mistake
    assert(!mmu_is_enabled());
    pin_mmu_enable();
    assert(mmu_is_enabled());

    // checking that lookup of user address
    // succeeds and of illegal does not.
    uint32_t res;
    if(!tlb_contains_va(&res, user_va_addr))
        panic("user address is missing\n");
    // make sure can't access illegal.
    if(tlb_contains_va(&res, SEG_ILLEGAL))
        panic("illegal address is mapped??\n");

    trace("MMU is on and working!\n");
    
    // 4. now turn on the MMU and make sure the expected
    //    entries are there.
    // now r/w user address.
    uint32_t x = GET32(user_va_addr);
    trace("asid 1 = got: %x\n", x);
    assert(x == 0xdeadbeef);
    PUT32(user_va_addr, 1);

    pin_mmu_disable();
    assert(!mmu_is_enabled());

    // check that the write was what we expected.
    trace("MMU is off!\n");
    trace("phys addr1=%x\n", GET32(user_pa_addr));
    assert(GET32(user_pa_addr) == 1);

    trace("SUCCESS!\n");
}
