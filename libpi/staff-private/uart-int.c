#include "rpi.h"
#include "rpi-interrupts.h"
#include "uart.h"
#include "uart-int.h"

// don't dereference directly!
static unsigned *const aux_irq = (void*)0x20215000;

unsigned *const uart_get_aux_irq(void) { return aux_irq; }

// smallish queue so we don't eat a lot of space: probably should
// use kmalloc.
#define CQ_N 2048
#include "libc/circular.h"
static cq_t getQ;
static cq_t putQ;
volatile unsigned n_uart_interrupts = 0;

static void or32(volatile void *addr, unsigned v) {
    dev_barrier();
    put32(addr, get32(addr) | v);
    dev_barrier();
}

static void OR32(unsigned addr, unsigned v) {
    return or32((void*)addr, v);
}

// ugh. my mistake.  we are dealing with the *miniUART*, not the *UART*.  thus
// the following is wrong:
//      WRONG: p 113: have to enable uart_int which = 57 (far right column of first table).
//      WRONG: This means we use enable_IRQs_2:
// we are instead enabling the aux interrupts.   this is 29.
// from p115
//  - Enable_IRQs_1 [offset 0x204] covers [0..31],  
//  - Enable_IRQs_2 [offset 0x208] covers [32..63]
static void uart_enable_gpu_int(void) {
    OR32(Enable_IRQs_1, 1 << 29);
}

void uart_disable_rx_int(hw_uart_t *uart) {
    or32(&uart->ier, get32(&uart->ier) & ~0b1);
}
void uart_enable_rx_int(hw_uart_t *uart) {
    or32(&uart->ier, 0b1);
}
void uart_disable_tx_int(hw_uart_t *uart) {
    put32(&uart->ier, get32(&uart->ier) & ~0b10);
}
void uart_enable_tx_int(hw_uart_t *uart) {
    or32(&uart->ier, 0b10);
}

int uart_tx_int_enabled(hw_uart_t *uart) {
    return (get32(&uart->ier) & 0b10) != 0;
}

// we do both rx and tx.
static void uart_enable_ints(struct aux_periphs *uart) {
    // XXX: the table header is incorrect!  the 2nd field is ier.
    // erratta: document on p12 says bit 1 = rx int, bit 0 = tx int
    // but these are reversed.   rx = bit 0, tx = bit 1.
    uart_enable_tx_int(uart);
    uart_enable_rx_int(uart);
}


// again: the table headings of p12/p13 are wrong.  
// all these use the 3rd word in the struct, which is iir.
//
// this checks and clears the interrupt.
uart_int_t uart_has_int(struct aux_periphs *uart) {
    dev_barrier();
    unsigned x = get32(&uart->iir);
    // p13: bit is clear when interrupt is pending
    if((x&1) == 1)
        return 0;

    // get bits 2:1
    unsigned v = (x>>1) & 0b11;
    demand(v != 0b11, impossible from broadcom page 13);
    demand(v != 0b00, impossible from broadcom page 13);

    if(v == 0b10)
        return UART_RX_INT;
    else if(v == 0b01) 
        return UART_TX_INT;
    else {
        panic("impossible value: %b\n", x);
    }
}


// i don't think we need to be more precise?
void uart_clear_int(hw_uart_t *uart) {
    dev_barrier();

    // p13: writing 0b110 will clear rx and tx fito
    // we also write a 0b1 to clear the int (if 
    // this works)
    or32(&uart->iir, 0b111);

    // uart and aux are two different devices.
    dev_barrier();

    put32(aux_irq, get32(aux_irq) & ~0b1);
    dev_barrier();
}

void uart_init_with_interrupts(void) {
    cq_init(&getQ,1);
    cq_init(&putQ,1);

    hw_uart_t *uart = uart_get();

    // already enabled?  maybe modify it to check.
    uart_init();
    uart_clear_int(uart);
    uart_enable_ints(uart);
    dev_barrier();
    uart_enable_gpu_int();
    dev_barrier();
}

// must be called with interrupts disabled.  add a check.
void uart_int_flush(void) {
    // check that ints are disabled.
    assert(!int_is_enabled());
    while(cq_nelem(&putQ) && uart_can_putc())
        uart_putc(cq_pop(&putQ));
}

// flush until all characters are gone from putQ
void uart_int_flush_all(void) {
    assert(int_is_enabled());
    system_disable_interrupts();
    // check that ints are disabled.
    while(cq_nelem(&putQ))
        uart_int_flush();
    system_enable_interrupts();
}


// called to handle an interrupt: how do you know what channel to deal with?
// i think you'd have to register the list of channels that can get interrupts,
// and call each one.   i think the esp must ensure that the message is delivered
// atomically, otherwise you don't know how it's mixed.   as a result, everything
// should just work(?)
int uart_interrupt_handler(void) {
    dev_barrier();
    hw_uart_t *uart = uart_get();

#if 1
    // actually: this might be good: if we get lots of GPU ints, could
    // be doing lots of useless work.

    // this does not work with tracing.
    // this is not even necessary: will be checking below.
    if(!uart_has_int(uart))
        return 0;
#endif

    n_uart_interrupts++;

    // actually: we don't even really care if there is an interrupt.  we just
    // see if there is space to move --- since it can free up at any time.

    // while the UART has received chars, move them to the getQ
    // do this first since they will vaporize under overflow.
    while(uart_has_data()) 
        cq_push(&getQ, uart_getc());

    // if we get here and there is no interrupt possible, 
    // we turn them off.
    if(!cq_nelem(&putQ))
        uart_disable_tx_int(uart);
    else
        uart_int_flush();
    dev_barrier();
    return 0;
}

int uart_has_data_int(void) { 
    return cq_nelem(&getQ) != 0; 
}

int uart_getc_int(void) {
    dev_barrier();
    while(!uart_has_data_int())
        ;
    return cq_pop(&getQ);
}

// we should make a version that returns the previous flag
// so you can do recursion?  or is it better to have a 
// simple counter? 

// strictly speaking, it's not an error to disable twice.
// you might be doing this to be conservative.  
// it is an error to enable twice.
// actually all the different methods have some suckiness.

// first do this as a polling setup.
int uart_putc_int(int c) {
    // must do this first.
    cq_push(&putQ, c);

    // if ints are not enabled, then the 
    // subsequent enable is going to cause problems!
    // need recursive.
    // ok, this does not work with tracing b/c of the assert.
#if 1
    assert(int_is_enabled());
    hw_uart_t *u = uart_get();
    // it'd be nice if we could start the dev barrier
    // and then make sure it finished later.
    dev_barrier();

    // right now, we overkill with disabling interrupts.
    // i think this is unneeded --- the two races and why
    // i think they are benign are discussed below.
    //
    // check if enabled: note the interrupt handler
    // could interrupt right after we load the value 
    // in u->iir, and turn it off.  we would think it's
    // enabled when it is not, and not re-enable.  however,
    // this only occurs when the putQ is empty, which
    // means it does not matter if we enable since there
    // is nothing to do.
    system_disable_interrupts();
    if(!uart_tx_int_enabled(u)) {
        // note: i think having a race here is fine: in the worst case
        // we will store a stale enable, which will trigger a spurious
        // interrupt.
        uart_enable_tx_int(u);
        dev_barrier();
    }
    system_enable_interrupts();
#else

    // can optimize all this
    system_disable_interrupts();

    dev_barrier();
    uart_int_flush();
    if(cq_nelem(&putQ))
        uart_enable_tx_int(uart_get());
    dev_barrier();

    system_enable_interrupts();
#endif
    return 1;
}
