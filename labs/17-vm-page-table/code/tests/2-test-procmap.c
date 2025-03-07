// first test do low level setup of everything.
#include "rpi.h"
#include "pt-vm.h"

void notmain(void) { 
    enum { OneMB = 1024*1024};
    // map the heap: for lab cksums must be at 0x100000.
    kmalloc_init_set_start((void*)OneMB, OneMB);
    assert(!mmu_is_enabled());


    enum { dom_kern = 1,
           // domain for user = 2
           dom_user = 2 };          

    // read write the first mb.
    uint32_t user_addr = OneMB * 16;
    assert((user_addr>>12) % 16 == 0);
    uint32_t phys_addr1 = user_addr;
    PUT32(phys_addr1, 0xdeadbeef);

    procmap_t p = procmap_default_mk(dom_kern);
    procmap_push(&p, pr_ent_mk(user_addr, OneMB, MEM_RW, dom_user));

    trace("about to enable\n");
    vm_pt_t *pt = vm_map_kernel(&p,1);

    assert(mmu_is_enabled());
    trace("MMU is on and working!\n");
    
    uint32_t x = GET32(user_addr);
    trace("asid 1 = got: %x\n", x);
    assert(x == 0xdeadbeef);
    PUT32(user_addr, 1);

    vm_mmu_disable();
    assert(!mmu_is_enabled());
    trace("MMU is off!\n");
    trace("phys addr1=%x\n", GET32(phys_addr1));
    assert(GET32(phys_addr1) == 1);

    trace("SUCCESS!\n");
}
