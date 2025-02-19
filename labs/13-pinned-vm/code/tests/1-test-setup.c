// make a simple user mapping.
#include "rpi.h"
#include "pinned-vm.h"
#include "mmu.h"

void notmain(void) { 
    assert(!mmu_is_enabled());

    enum { OneMB = 1024*1024};
    // map the heap: for lab cksums must be at 0x100000.
    kmalloc_init_set_start((void*)OneMB, OneMB);

    staff_pin_mmu_init(~0);

    enum { 
        dom_kern = 1, // domain id for kernel
        dom_user = 2  // domain id for user
    };          

    uint32_t no_user = perm_rw_priv;

    // device memory: kernel domain, no user access, 
    // memory is strongly ordered, not shared.
    pin_t dev  = pin_mk_global(dom_kern, no_user, MEM_device);
    // kernel memory: same, but is only uncached.
    pin_t kern = pin_mk_global(dom_kern, no_user, MEM_uncached);

    // index into the 8 pinned entries in tlb.
    unsigned idx = 0;
    // map all device memory to itself.  ("identity map")
    pin_mmu_sec(idx++, 0x20000000, 0x20000000, dev);   // 0
    pin_mmu_sec(idx++, 0x20100000, 0x20100000, dev);   // 1
    pin_mmu_sec(idx++, 0x20200000, 0x20200000, dev);   // 2

    // map first two MB for the kernel (it would be better to not
    // have 0 (null) mapped of course, but our code starts at 0x8000.
    pin_mmu_sec(idx++, 0, 0, kern);                    // 3
    pin_mmu_sec(idx++, OneMB, OneMB, kern);            // 4

    // map kernel stack (or nothing will work)
    pin_mmu_sec(idx++, STACK_ADDR-OneMB, STACK_ADDR-OneMB, kern);   // 5

    // we don't take faults so don't need an exception stack.
    // pin_mmu_sec(idx++, INT_STACK_ADDR-OneMB, INT_STACK_ADDR-OneMB, kern);

    // create a single user entry, also uncached.
    enum { ASID1 = 1 };

    // non-global entry with user permissions. user attribute: not global.  
    pin_t user1 = pin_mk_user(dom_kern, ASID1, no_user, MEM_uncached); // 6
    
    // read/write the first mb.
    uint32_t user_addr = OneMB * 16;
    assert((user_addr>>12) % 16 == 0);
    uint32_t phys_addr1 = user_addr;
    PUT32(phys_addr1, 0xdeadbeef);

    // user mapping: do an ident.
    pin_mmu_sec(idx++, user_addr, phys_addr1, user1);
    assert(idx<8);

    trace("about to enable\n");

    lockdown_print_entries("about to turn on first time");
    staff_pin_mmu_switch(0,ASID1);
    pin_mmu_enable();

    // staff_mmu_on_first_time(1, null_pt);

    // checking that we can access.
    uint32_t res;
    if(tlb_contains_va(&res, MB*10))
        panic("how contain?\n");
    else
        output("success?\n");

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
