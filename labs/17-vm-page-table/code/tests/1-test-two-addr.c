// test that we can:
//   1. create private virt->phys mappings
//   2. switch between virtual address spaces
// by:
//   1. create two address spaces (ASID1, ASID2)
//   2. map the same virtual address <user_addr> in both
//      to different physical segments (<phys_addr1> and
//      <phys_addr2>.
//   3. switch between them reading and writing
#include "rpi.h"
#include "pt-vm.h"
#include "mmu.h"
#include "memmap-default.h"
#include "full-except.h"

void notmain(void) { 
    // map the heap: for lab cksums must be at 0x100000.
    kmalloc_init_set_start((void*)MB(1), MB(1));
    full_except_install(0);
    assert(!mmu_is_enabled());

    // default domain bits.
    vm_pt_t *pt1 = vm_pt_alloc(PT_LEVEL1_N);
    vm_mmu_init(dom_bits);
    assert(!mmu_is_enabled());

    pin_t kern = pin_mk_global(dom_kern, no_user, MEM_uncached);
    vm_map_sec(pt1, SEG_CODE, SEG_CODE, kern);
    vm_map_sec(pt1, SEG_HEAP, SEG_HEAP, kern);
    vm_map_sec(pt1, SEG_STACK, SEG_STACK, kern);
    vm_map_sec(pt1, SEG_INT_STACK, SEG_INT_STACK, kern);

    pin_t dev  = pin_mk_global(dom_kern, no_user, MEM_device);
    vm_map_sec(pt1, SEG_BCM_0, SEG_BCM_0, dev);
    vm_map_sec(pt1, SEG_BCM_1, SEG_BCM_1, dev);
    vm_map_sec(pt1, SEG_BCM_2, SEG_BCM_2, dev);

    // we have all the global mappings: dup.
    vm_pt_t *pt2 = vm_dup(pt1);

    enum { ASID1 = 1, ASID2 = 2 };
    // do a non-ident map
    enum {
        user_addr = MB(16),
        phys_addr1 = user_addr+MB(1),
        phys_addr2 = user_addr+MB(2)
    };
    pin_t user1 = pin_mk_user(dom_kern, ASID1, no_user, MEM_uncached);
    pin_t user2 = pin_mk_user(dom_kern, ASID2, no_user, MEM_uncached);
    vm_map_sec(pt1, user_addr, phys_addr1, user1);

    vm_map_sec(pt2, user_addr, phys_addr2, user2);

    // initialize with MMU off.
    PUT32(phys_addr1, 0x11111111);
    PUT32(phys_addr2, 0x22222222);

    trace("about to enable\n");
    trace("about to run ASID=%d\n", ASID1);

    // *****************************************************
    // turn on address space 1 and check we can write.

    vm_mmu_switch(pt1,0x140e,ASID1);
    vm_mmu_enable();
        trace("MMU is on and working!\n");
    
        uint32_t x = GET32(user_addr);
        trace("ASID %d = got: %x\n", ASID1, x);
        assert(x == 0x11111111);
        PUT32(user_addr, ASID1);

    vm_mmu_disable();

    // check that the write happened.
    trace("phys addr1=%x\n", GET32(phys_addr1));
    assert(GET32(phys_addr1) == ASID1);


    // *****************************************************
    // switch to 2nd address space and check we can write.
    vm_mmu_switch(pt2,0x140e,ASID2);
    vm_mmu_enable();
        x = GET32(user_addr);
        trace("asid %d = got: %x\n", ASID2, x);
        assert(x == 0x22222222);
        PUT32(user_addr, ASID2);
    vm_mmu_disable();

    trace("phys addr2=%x\n", GET32(phys_addr2));
    assert(GET32(phys_addr2) == ASID2);

    // *****************************************************
    // now check that works by switching even with MMU on.

    trace("about to check that can switch ASID w/ MMU on\n");
    PUT32(phys_addr1, 0x11111111); // reset 
    PUT32(phys_addr2, 0x22222222);

    vm_mmu_switch(pt1,0x140e,ASID1);
    vm_mmu_enable();
    assert(GET32(user_addr) == 0x11111111);
    PUT32(user_addr, ASID1);

    vm_mmu_switch(pt2,0x140e,ASID2);
    assert(GET32(user_addr) == 0x22222222);
    PUT32(user_addr, ASID2);

    vm_mmu_disable();

    // make sure that the writes happened.
    assert(GET32(phys_addr1) == ASID1);
    assert(GET32(phys_addr2) == ASID2);
    trace("SUCCESS!\n");
}
