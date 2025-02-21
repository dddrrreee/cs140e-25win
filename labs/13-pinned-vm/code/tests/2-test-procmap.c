// simple example of using procmap.  replicates <1-test-one-addr.c>
#include "rpi.h"
#include "procmap.h"

void notmain(void) { 
    kmalloc_init_set_start((void*)SEG_HEAP, MB(1));

    // read write the first mb.
    uint32_t user_addr = MB(16);
    assert((user_addr>>12) % 16 == 0);
    uint32_t phys_addr1 = user_addr;
    PUT32(phys_addr1, 0xdeadbeef);

    procmap_t p = procmap_default_mk(dom_kern);
    procmap_push(&p, pr_ent_mk(user_addr, MB(1), MEM_RW, dom_user));

    trace("about to enable\n");
    procmap_pin_on(&p);

    lockdown_print_entries("about to turn on first time");
    assert(mmu_is_enabled());
    trace("MMU is on and working!\n");
    
    uint32_t x = GET32(user_addr);
    trace("asid 1 = got: %x\n", x);
    assert(x == 0xdeadbeef);
    PUT32(user_addr, 1);

    pin_mmu_disable();
    assert(!mmu_is_enabled());
    trace("MMU is off!\n");
    trace("phys addr1=%x\n", GET32(phys_addr1));
    assert(GET32(phys_addr1) == 1);

    trace("SUCCESS!\n");
}
