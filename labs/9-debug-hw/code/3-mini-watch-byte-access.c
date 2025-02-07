// test of:
//   1. the debug interface (see debug.c)
//   2. that you can trap on sub-word read/writes 

#include "rpi.h"
#include "mini-watch.h"

// set these before each fault so we can check that the right 
// thing happened.
static volatile uint32_t expected_fault_addr;
static volatile uint32_t expected_fault_pc;
static volatile uint32_t load_fault_n, store_fault_n;

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
        output("load fault at pc=%x, addr=%x\n", w->fault_pc, w->fault_addr);
        assert(w->fault_pc == (uint32_t)GET8);
        load_fault_n++;
    } else {
        output("store fault at pc=%x, addr=%x\n", w->fault_pc, w->fault_addr);
        assert(w->fault_pc == (uint32_t)PUT8);
        store_fault_n++;
    }
    mini_watch_disable(w->fault_addr);
}

//test that watchpoint works:
//  1. set a watchpoint on <fault_addr>, 
//  2. do a PUT8(fault_addr), check result.
//  3. do a GET8(fault_addr), check result.
static void test_bad_addr(uint32_t fault_addr) {
    static uint8_t val = 0xfa;
    uint8_t got = 0;

    // keep changing the value from run to run
    val = val*val;

    expected_fault_addr = fault_addr;
    trace("about to test addr %x\n", fault_addr);

    // one-time setup for this address: 
    //  install the watchpoint handler.
    mini_watch_addr((void*)fault_addr);
    assert(mini_watch_enabled());
    trace("set watchpoint for addr %p\n", fault_addr);

    trace("should see a store fault!\n");
    expected_fault_pc = (uint32_t)PUT8;
    PUT8(fault_addr,val);
    assert(!mini_watch_enabled());

    if(!store_fault_n)
        panic("did not see a store fault\n");

    got = GET8(fault_addr);
    if(got != val)
        panic("stored %x, but got %x\n", val, got);

    // since already watching <fault_addr> just re-enable.
    mini_watch_addr((void*)fault_addr);
    assert(mini_watch_enabled());

    trace("should see a load fault!\n");
    expected_fault_pc = (uint32_t)GET8;
    got = GET8(fault_addr);
    if(!load_fault_n)
        panic("did not see a load fault\n");
    if(got != val)
        panic("stored %x, but got %x\n", val, got);
}

void notmain(void) {
    mini_watch_init(watchpt_handler, 0);

    for(int i = 0; i < 4; i++)
        test_bad_addr(i);

    trace("SUCCESS\n");
}
