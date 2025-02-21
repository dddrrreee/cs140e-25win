// complete working example of trivial vm:
//  1. identity map the kernel (code, heap, kernel stack, 
//     exception stack, device memory) using 1MB segments.
//  2. turn VM on/off making sure we can print.
//  3. with VM on, write to an unmapped address and make sure we 
//     get a fault.
//
//  the routines that start with "pin_" are what you'll build today,
//  the ones with with "staff_mmu_" are low level hardware 
//  routines that you'll build next VM lab.
//
//  there are some page numbers: if they start with a "B" they
//  are from the armv6 pdf, if with a number, from arm1176 pdf
#include "rpi.h"
#include "pinned-vm.h"
#include "full-except.h"
#include "memmap.h"

#define MB(x) ((x)*1024*1024)

// These are the default segments (segment = one MB)
// that need to be mapped for our binaries so far
// this quarter. 
//
// these will hold for all our tests today.
//
// if we just map these segments we will get faults
// for stray pointer read/writes outside of this region.
enum {
    // code starts at 0x8000, so map the first MB
    //
    // if you look in <libpi/memmap> you can see
    // that all the data is there as well, and we have
    // small binaries, so this will cover data as well.
    //
    // NOTE: obviously it would be better to not have 0 (null) 
    // mapped, but our code starts at 0x8000 and we are using
    // 1mb sections (which require 1MB alignment) so we don't
    // have a choice unless we do some modifications.  
    //
    // you can fix this problem as an extension: very useful!
    SEG_CODE = MB(0),

    // as with previous labs, we initialize 
    // our kernel heap to start at the first 
    // MB. it's 1MB, so fits in a segment. 
    SEG_HEAP = MB(1),

    // if you look in <staff-start.S>, our default
    // stack is at STACK_ADDR, so subtract 1MB to get
    // the stack start.
    SEG_STACK = STACK_ADDR - MB(1),

    // the interrupt stack that we've used all class.
    // (e.g., you can see it in the <full-except-asm.S>)
    // subtract 1MB to get its start
    SEG_INT_STACK = INT_STACK_ADDR - MB(1),

    // the base of the BCM device memory (for GPIO
    // UART, etc).  Three contiguous MB cover it.
    SEG_BCM_0 = 0x20000000,
    SEG_BCM_1 = SEG_BCM_0 + MB(1),
    SEG_BCM_2 = SEG_BCM_0 + MB(2),

    // we guarantee this (2MB) is an 
    // unmapped address
    SEG_ILLEGAL = MB(2),
};

// used to store the illegal address we will write.
static volatile uint32_t illegal_addr;

// macros to generate inline assembly routines.  recall
// we've used these for the debug lab, etc.
#include "asm-helpers.h"

// b4-44: get the fault address from a data abort.
cp_asm_get(cp15_fault_addr, p15, 0, c6, c0, 0)

// a trivial data abort handler that checks that we 
// faulted on the address that we expected.
//   - note: bad form that we don't actually check 
//     that its a data abort.
static void data_abort_handler(regs_t *r) {
    uint32_t fault_addr;

#if 0
    // b4-44
    // alternatively you can use the inline assembly raw.
    // can be harder to debug.
    asm volatile("MRC p15, 0, %0, c6, c0, 0" : "=r" (fault_addr));
#else
    fault_addr = cp15_fault_addr_get();
#endif

    // make sure we faulted on the address that should be accessed.
    if(fault_addr != illegal_addr)
        panic("illegal fault!  expected %x, got %x\n",
            illegal_addr, fault_addr);
    else
        trace("SUCCESS!: got a fault on address=%x\n", fault_addr);

    // done with test.
    trace("all done: going to reboot\n");
    clean_reboot();
}

void notmain(void) { 
    // ******************************************************
    // 1. one-time initialization.
    //   - create an empty page table (to catch errors).
    //   - setup exception handlers.
    //   - compute permissions for no-user-access.

    // initialize the heap.  as stated above,
    // starts at 1MB, and is 1MB big.
    kmalloc_init_set_start((void*)SEG_HEAP, MB(1));

    // example sanity check that entire process fits 
    // within 1MB segment: these symbols are in <memmap.h> 
    // and defined in <libpi/memmap> (we've used in previous
    // labs).
    assert((uint32_t)__prog_end__ < SEG_CODE + MB(1));
    assert((uint32_t)__code_start__ >= SEG_CODE);

    // setup a data abort handler (just like last lab).
    full_except_install(0);
    full_except_set_data_abort(data_abort_handler);

    // allocate a page table with invalid
    // entries.
    //
    // we will TLB pin all valid mappings.  
    // if the code is correct, the hardware
    // will never look anything up in the page
    // table.
    // 
    // however, if the code is buggy and does a 
    // a wild memory access that isn't in any
    // pinnned entry, the hardware would then try 
    // to look the address up in the page table
    // pointed to by the tlbwr0 register.
    //
    // if this page table isn't explicitly
    // initialized to invalid entries, the hardware
    // would interpret the garbage bits there 
    // valid and potentially insert them (very
    // hard bug to find).
    //
    // to prevent this we allocate a zero-filled 
    // page table.
    //  - 4GB/1MB section * 4 bytes = 4096*4.
    //  - zero-initialized will set each entry's 
    //    valid bit to 0 (invalid).
    //  - lower 14 bits (16k aligned) as required
    //    by the hardware.
    void *null_pt = kmalloc_aligned(4096*4, 1<<14);
    assert((uint32_t)null_pt % (1<<14) == 0);

    // in <mmu.h>: checks cp15 control reg 1.
    //  - will implement in next vm lab.
    assert(!mmu_is_enabled());

    // no access for user (r/w privileged only)
    // defined in <mem-attr.h>.  
    //
    // is APX and AP fields bit-wise or'd
    // together: (APX << 2 | AP) see 
    //  - 3-151 for table, or B4-9
    enum { no_user = perm_rw_priv };

    // armv6 has 16 different domains with their own privileges.
    // just pick one for the kernel.
    enum { dom_kern = 1 };

    // attribute for device memory (see <pin.h>).  this 
    // is needed when pinning device memory:
    //   - permissions: kernel domain, no user access, 
    //   - memory rules: strongly ordered, not shared.
    //     see <mem-attr.h> for <MEM_device>
    pin_t dev  = pin_mk_global(dom_kern, no_user, MEM_device);

    // attribute for kernel memory (see <pin.h>)
    //   - protection: same as device; we don't want the
    //     user to read/write it.
    //   - memory rules: uncached access.  you can start
    //     messing with this for speed, though have to 
    //     do cache coherency maintance
    pin_t kern = pin_mk_global(dom_kern, no_user, MEM_uncached);

    // Q1: if you uncomment this, what happens / why?
    // kern = pin_mk_global(dom_kern, perm_ro_priv, MEM_uncached);

    // ******************************************************
    // 2. map the process's memory.  We use the enums
    //    above that define where everything is.

    // <mmu.h>: one-time initialize of MMU hardware
    //  - invalidate TLB, caches, etc.
    //  - will implement in next vm lab.
    staff_mmu_init();

    // identiy map all segments in one of the available
    // 0..7 TLB pinned entries.
    pin_mmu_sec(0, SEG_CODE, SEG_CODE, kern);
    pin_mmu_sec(1, SEG_HEAP, SEG_HEAP, kern); 
    // Q2: if you comment this out, what happens?
    pin_mmu_sec(2, SEG_STACK, SEG_STACK, kern); 
    // Q3: if you comment this out, what happens?
    pin_mmu_sec(3, SEG_INT_STACK, SEG_INT_STACK, kern); 

    // map all device memory to itself.  ("identity map")
    pin_mmu_sec(4, SEG_BCM_0, SEG_BCM_0, dev); 
    pin_mmu_sec(5, SEG_BCM_1, SEG_BCM_1, dev); 
    pin_mmu_sec(6, SEG_BCM_2, SEG_BCM_2, dev); 

    // ******************************************************
    // 3. setup virtual address context.
    //  - domain permissions.
    //  - page table, asid, pid.

    // b4-42: give permissions for all domains.
    // Q4: if you set this to ~0, what happens w.r.t. Q1?
    // Q5: if you set this to 0, what happens?
    staff_domain_access_ctrl_set(DOM_client << dom_kern*2); 

    // set address space id, page table, and pid.
    // note:
    //  - <pid> is ignored by the hw: it's just to help the os.
    //  - <asid> doesn't matter for this test b/c all entries 
    //    are global.
    //  - recall the page table has all entries invalid and is
    //    just to catch memory errors.
    enum { ASID = 1, PID = 128 };
    staff_mmu_set_ctx(PID, ASID, null_pt);

    // if you want to see the lockdown entries.
    // lockdown_print_entries("about to turn on first time");

    // ******************************************************
    // 4. turn MMU on/off, checking that it worked.
    trace("about to enable\n");
    for(int i = 0; i < 10; i++) {
        assert(!staff_mmu_is_enabled());
        // mmu.h: you'll implement next vm lab.  
        staff_mmu_enable();

        // this uses: stack, code, data, BCM.
        if(staff_mmu_is_enabled())
            trace("MMU ON: hello from virtual memory!  cnt=%d\n", i);
        else
            panic("MMU is not on?\n");

        // mmu.h: you'll implement next vm lab.  
        staff_mmu_disable();
        assert(!staff_mmu_is_enabled());
        trace("MMU is off!\n");
    }

    // ******************************************************
    // 5. setup exception handling and make sure we get a fault.
    // 
    //    you should make a copy of this test case, and 

    // we write to an address we know is unmapped so we get
    // a fault.
    illegal_addr = SEG_ILLEGAL;

    // this <PUT32> should "work" since vm is off.
    assert(!staff_mmu_is_enabled());
    PUT32(illegal_addr, 0xdeadbeef);
    trace("we wrote without vm: got %x\n", GET32(illegal_addr));
    assert(GET32(illegal_addr) == 0xdeadbeef);

    // now try with MMU on: it's a good exercise to make a copy
    // of this test case and change it so you repeatedly fault.
    staff_mmu_enable();
    assert(staff_mmu_is_enabled());
    output("about to write to <%x>\n", illegal_addr);
    // this <PUT32> should cause a data_abort.
    PUT32(illegal_addr, 0xdeadbeef);
    panic("should not reach here\n");
}
