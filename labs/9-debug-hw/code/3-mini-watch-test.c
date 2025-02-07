// handle a store to address 0 (null)
#include "rpi.h"
#include "mini-watch.h"

static volatile uint32_t load_fault_n, store_fault_n;
static volatile uint32_t expected_fault_addr;
static volatile uint32_t expected_fault_pc;

// change to passing in the saved registers.
static void 
watchpt_handler(void *data, watch_fault_t *w) {
    if(w->fault_addr != (void*)expected_fault_addr)
        panic("expected watchpt fault on addr %p, have %p\n",
                        expected_fault_addr, w->fault_addr);
    if(w->fault_pc != expected_fault_pc)
        panic("expected watchpt fault at pc=%p, faulted at=%p\n",
                        expected_fault_pc, w->fault_pc);

    if(w->is_load_p) {
        trace("load fault at pc=%x\n", w->fault_pc);
        assert(w->fault_pc == (uint32_t)GET32);
        load_fault_n++;
    } else {
        trace("store fault at pc=%x\n", w->fault_pc);
        assert(w->fault_pc == (uint32_t)PUT32);
        store_fault_n++;
    }
    mini_watch_disable(w->fault_addr);
}

void notmain(void) {
    mini_watch_init(watchpt_handler, 0);

    enum { null = 0, val = 0xdeadbeef };
    mini_watch_addr(null);

    assert(mini_watch_enabled());
    trace("set watchpoint for addr %p\n", null);

    expected_fault_addr = null;
    expected_fault_pc = (uint32_t)PUT32;

    trace("should see a store fault!\n");
    PUT32(null,val);

    if(!store_fault_n)
        panic("did not see a store fault\n");

    assert(!mini_watch_enabled());
    uint32_t got = GET32(null);
    if(got != val)
        panic("expected GET(%x)=%x, have %x\n", null, val, got);

    mini_watch_addr(null);
    assert(mini_watch_enabled());

    trace("should see a load fault!\n");
    expected_fault_addr = null;
    expected_fault_pc = (uint32_t)GET32;

    got = GET32(null);
    if(!load_fault_n)
        panic("did not see a load fault\n");

    if(got != val)
        panic("expected GET(%x)=%x, have %x\n", null, val, got);
    
    trace("SUCCESS\n");
}
